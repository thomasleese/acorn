//
// Created by Thomas Leese on 12/01/2017.
//

#include <iostream>
#include <set>
#include <sstream>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/symboltable/namespace.h"
#include "acorn/symboltable/symbol.h"
#include "acorn/typesystem/types.h"
#include "acorn/utils.h"

#include "acorn/typesystem/inferrer.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::typesystem;

static auto logger = spdlog::get("acorn");

TypeInferrer::TypeInferrer(symboltable::Namespace *scope) {
    push_scope(scope);
}

typesystem::TypeType *TypeInferrer::find_type_constructor(ast::Node *node, std::string name) {
    auto symbol = scope()->lookup(this, node, name);
    return_null_if_null(symbol);
    return dynamic_cast<typesystem::TypeType *>(symbol->type());
}

typesystem::TypeType *TypeInferrer::find_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters) {
    std::vector<typesystem::Type *> parameterTypes;

    for (auto parameter : parameters) {
        visit(parameter);
        parameterTypes.push_back(parameter->type());
    }

    typesystem::TypeType *typeConstructor = find_type_constructor(node, name);
    if (typeConstructor != nullptr) {
        return typeConstructor->with_parameters(parameterTypes);
    } else {
        report(InvalidTypeConstructor(node));
        return nullptr;
    }
}

typesystem::TypeType *TypeInferrer::find_type(ast::Node *node, std::string name) {
    return find_type(node, name, std::vector<ast::Name *>());
}

typesystem::TypeType *TypeInferrer::find_type(ast::Name *type) {
    std::vector<ast::Name *> parameters;
    for (auto &p : type->parameters()) {
        parameters.push_back(p.get());
    }

    return find_type(type, type->value(), parameters);
}

typesystem::Type *TypeInferrer::instance_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters) {
    auto type_constructor = find_type(node, name, parameters);
    if (type_constructor == nullptr) {
        return nullptr;
    }

    return type_constructor->create(this, node);
}

typesystem::Type *TypeInferrer::instance_type(ast::Node *node, std::string name) {
    return instance_type(node, name, std::vector<ast::Name *>());
}

typesystem::Type *TypeInferrer::instance_type(ast::Name *name) {
    std::vector<ast::Name *> parameters;
    for (auto &p : name->parameters()) {
        parameters.push_back(p.get());
    }

    return instance_type(name, name->value(), parameters);
}

typesystem::Type *TypeInferrer::builtin_type_from_name(ast::Name *node) {
    auto name = node->value();

    if (name == "Void") {
        return new typesystem::VoidType();
    } else if (name == "Bool") {
        return new typesystem::BooleanType();
    } else if (name == "Int8") {
        return new typesystem::IntegerType(8);
    } else if (name == "Int16") {
        return new typesystem::IntegerType(16);
    } else if (name == "Int32") {
        return new typesystem::IntegerType(32);
    } else if (name == "Int64") {
        return new typesystem::IntegerType(64);
    } else if (name == "Int128") {
        return new typesystem::IntegerType(128);
    } else if (name == "UInt8") {
        return new typesystem::UnsignedIntegerType(8);
    } else if (name == "UInt16") {
        return new typesystem::UnsignedIntegerType(16);
    } else if (name == "UInt32") {
        return new typesystem::UnsignedIntegerType(32);
    } else if (name == "UInt64") {
        return new typesystem::UnsignedIntegerType(64);
    } else if (name == "UInt128") {
        return new typesystem::UnsignedIntegerType(128);
    } else if (name == "Float16") {
        return new typesystem::FloatType(16);
    } else if (name == "Float32") {
        return new typesystem::FloatType(32);
    } else if (name == "Float64") {
        return new typesystem::FloatType(64);
    } else if (name == "Float128") {
        return new typesystem::FloatType(128);
    } else if (name == "UnsafePointer") {
        return new typesystem::UnsafePointerType();
    } else if (name == "Function") {
        return new typesystem::FunctionType();
    } else if (name == "Method") {
        return new typesystem::MethodType();
    } else if (name == "Tuple") {
        return new typesystem::TupleType();
    } else if (name == "Type") {
        return new typesystem::TypeDescriptionType();
    } else {
        logger->critical("Unknown builtin type: {}", name);
        return nullptr;
    }
}

bool TypeInferrer::infer_call_type_parameters(ast::Call *call, std::vector<typesystem::Type *> parameter_types, std::vector<typesystem::Type *> argument_types) {
    assert(parameter_types.size() == argument_types.size());

    int i = 0;
    for (auto t : parameter_types) {
        auto inferred_type_parameters = call->inferred_type_parameters();

        auto dt = dynamic_cast<typesystem::Parameter *>(t);
        if (dt != nullptr) {
            auto it = inferred_type_parameters.find(dt->type());
            if (it != inferred_type_parameters.end() && !it->second->is_compatible(argument_types[i])) {
                report(TypeMismatchError(call, argument_types[i], it->second));
                return false;
            } else {
                call->add_inferred_type_parameter(dt->type(), argument_types[i]);
            }
        } else {
            auto dt2 = dynamic_cast<typesystem::ParameterType *>(t);
            if (dt2 != nullptr) {
                auto arg = dynamic_cast<typesystem::TypeType *>(argument_types[i]);

                auto it = inferred_type_parameters.find(dt2);
                if (it != inferred_type_parameters.end() && !it->second->is_compatible(argument_types[i])) {
                    report(TypeMismatchError(call, argument_types[i], it->second));
                    return false;
                } else {
                    call->add_inferred_type_parameter(dt2, arg->create(this, call));
                }
            } else {
                if (!infer_call_type_parameters(call, t->parameters(), argument_types[i]->parameters())) {
                    return false;
                }
            }
        }

        i++;
    }

    return true;
}

typesystem::Type *TypeInferrer::replace_type_parameters(typesystem::Type *type, std::map<typesystem::ParameterType *, typesystem::Type *> replacements) {
    auto parameter = dynamic_cast<typesystem::Parameter *>(type);
    if (parameter != nullptr) {
        auto it = replacements.find(parameter->type());
        assert(it != replacements.end());
        type = it->second;
    }

    assert(type);

    auto parameters = type->parameters();

    if (parameters.empty()) {
        return type;
    }

    for (size_t i = 0; i < parameters.size(); i++) {
        parameters[i] = replace_type_parameters(parameters[i], replacements);
    }

    return type->with_parameters(parameters);
}

ast::Node *TypeInferrer::visit_block(ast::Block *node) {
    ast::Visitor::visit_block(node);

    auto &expressions = node->expressions();

    if (expressions.empty()) {
        node->set_type(new typesystem::Void());
    } else {
        node->copy_type_from(expressions.back().get());
    }

    return node;
}

ast::Node *TypeInferrer::visit_name(ast::Name *node) {
    auto symbol = scope()->lookup(this, node);
    return_null_if_null(symbol);

    if (dynamic_cast<typesystem::TypeType *>(symbol->type()) != nullptr) {
        node->set_type(find_type(node));
    } else {
        // it *must* be empty
        assert(node->parameters.empty());
        node->set_type(symbol->type());
    }

    return node;
}

ast::Node *TypeInferrer::visit_variable_declaration(ast::VariableDeclaration *node) {
    visit(node->name().get());

    auto symbol = scope()->lookup(this, node->name().get());

    if (node->given_type()) {
        visit(node->given_type().get());

        node->set_type(instance_type(node->given_type().get()));
        symbol->copy_type_from(node);
    }

    return node;
}

ast::Node *TypeInferrer::visit_int(ast::Int *node) {
    node->set_type(instance_type(node, "Int64"));
    return node;
}

ast::Node *TypeInferrer::visit_float(ast::Float *node) {
    node->set_type(instance_type(node, "Float64"));
    return node;
}

ast::Node *TypeInferrer::visit_complex(ast::Complex *node) {
    node->set_type(instance_type(node, "Complex"));
    return node;
}

ast::Node *TypeInferrer::visit_string(ast::String *node) {
    node->set_type(instance_type(node, "String"));
    return node;
}

ast::Node *TypeInferrer::visit_list(ast::List *node) {
    ast::Visitor::visit_list(node);

    auto &elements = node->elements();

    std::vector<typesystem::Type *> types;
    for (auto &element : elements) {
        bool inList = false;
        for (auto type : types) {
            if (type->is_compatible(element->type())) {
                inList = true;
                break;
            }
        }

        if (!inList) {
            types.push_back(element->type());
        }
    }

    if (types.size() == 1) {
        std::vector<typesystem::Type *> p;
        p.push_back(types[0]);
        auto array_type = find_type(node, "Array")->with_parameters(p);
        node->set_type(array_type->create(this, node));
    } else {
        // FIXME show error
        node->set_type(nullptr);
    }

    return node;
}

ast::Node *TypeInferrer::visit_tuple(ast::Tuple *node) {
    ast::Visitor::visit_tuple(node);

    std::vector<typesystem::Type *> element_types;

    for (auto &element : node->elements()) {
        // FIXME check has type
        element_types.push_back(element->type());
    }

    node->set_type(new typesystem::Tuple(element_types));

    return node;
}

ast::Node *TypeInferrer::visit_dictionary(ast::Dictionary *node) {
    report(TypeInferenceError(node));
    return node;
}

ast::Node *TypeInferrer::visit_call(ast::Call *node) {
    visit(node->operand().get());
    return_null_if_null_type(node->operand());

    for (auto &argument : node->positional_arguments()) {
        visit(argument.get());
        return_null_if_null_type(argument);
    }

    for (auto &entry : node->keyword_arguments()) {
        visit(entry.second.get());
        return_null_if_null_type(entry.second);
    }

    auto function = dynamic_cast<typesystem::Function *>(node->operand_type());
    if (function == nullptr) {
        node->set_type(new typesystem::Function());
        report(TypeMismatchError(node->operand().get(), node));
        delete node->type();
        node->set_type(nullptr);
        // FIXME make the construct accept a type directly
        return nullptr;
    }

    auto method = function->find_method(node);
    if (method == nullptr) {
        std::stringstream ss;
        ss << "Method not found for these types:\n";
        for (auto &argument : node->positional_arguments()) {
            ss << argument->type()->name() << ", ";
        }

        ss << "\nAvailable methods are:\n";
        for (auto method : function->methods()) {
            ss << " - " << method->name() << "\n";
        }
        ss << function->no_methods() << " methods.";

        report(UndefinedError(node, ss.str()));

        return nullptr;
    }

    node->set_method_index(function->index_of(method));

    if (method->is_generic()) {
        if (!infer_call_type_parameters(node, method->parameter_types(), method->ordered_argument_types(node))) {
            logger->critical("Could not infer type parameters.");
            return nullptr;
        }

        auto return_type = replace_type_parameters(
                method->return_type(), node->inferred_type_parameters()
        );

        node->set_method_specialisation_index(method->no_generic_specialisation());
        method->add_generic_specialisation(node->inferred_type_parameters());

        node->set_type(return_type);
    } else {
        node->set_type(method->return_type());
    }

    return node;
}

ast::Node *TypeInferrer::visit_ccall(ast::CCall *node) {
    ast::Visitor::visit_ccall(node);

    for (auto &param : node->parameters()) {
        param->set_type(instance_type(param.get()));
    }

    // TODO check arg and param typesystem match

    node->set_type(instance_type(node->given_return_type().get()));

    return node;
}

ast::Node *TypeInferrer::visit_cast(ast::Cast *node) {
    ast::Visitor::visit_cast(node);
    node->set_type(instance_type(node->new_type().get()));
    return node;
}

ast::Node *TypeInferrer::visit_assignment(ast::Assignment *node) {
    auto symbol = scope()->lookup(this, node->lhs()->name().get());
    return_null_if_null(symbol);

    auto rhs = node->rhs().get();

    if (!node->builtin()) {
        visit(rhs);
        return_null_if_null_type(rhs);
    }

    auto lhs = node->lhs().get();

    visit(lhs);
    if (!lhs->has_type()) {
        lhs->copy_type_from(node->rhs());
    }

    return_null_if_null_type(lhs);

    if (!node->builtin()) {
        if (!lhs->has_compatible_type_with(rhs)) {
            report(TypeMismatchError(lhs, rhs));
            return nullptr;
        }
    }

    node->copy_type_from(lhs);
    symbol->copy_type_from(node);

    return node;
}

ast::Node *TypeInferrer::visit_selector(ast::Selector *node) {
    if (!node->operand()) {
        return node;
    }

    auto operand = node->operand().get();

    visit(operand);
    return_null_if_null_type(operand);

    auto module_type = dynamic_cast<typesystem::ModuleType *>(operand->type());
    auto record_type = dynamic_cast<typesystem::RecordType *>(operand->type());
    auto record = dynamic_cast<typesystem::Record *>(operand->type());

    if (module_type != nullptr) {
        auto module_name = static_cast<ast::Name *>(operand);

        auto symbol = scope()->lookup(this, module_name);
        return_null_if_null(symbol);

        auto child_symbol = symbol->scope()->lookup(this, node->field().get());
        return_null_if_null(child_symbol);

        node->set_type(child_symbol->type());
    } else if (record_type != nullptr) {
        if (node->field()->value() == "new") {
            node->set_type(record_type->constructor());
        } else {
            report(UndefinedError(node->field().get(), "new"));
        }
    } else if (record != nullptr) {
        auto field = node->field().get();

        auto field_type = record->child_type(field->value());
        if (field_type != nullptr) {
            node->set_type(field_type);
        } else {
            report(UndefinedError(field));
        }
    } else {
        report(TypeMismatchError(operand, operand->type()->name(), "module, record type or record"));
    }

    return node;
}

ast::Node *TypeInferrer::visit_while(ast::While *node) {
    Visitor::visit_while(node);

    node->copy_type_from(node->body());

    return node;
}

ast::Node *TypeInferrer::visit_if(ast::If *node) {
    Visitor::visit_if(node);

    // FIXME return a union type
    node->copy_type_from(node->true_case());

    return node;
}

ast::Node *TypeInferrer::visit_return(ast::Return *node) {
    auto expression = node->expression().get();

    visit(expression);
    return_null_if_null_type(expression)

    if (!m_function_stack.empty()) {
        auto def = m_function_stack.back();
        auto method = static_cast<typesystem::Method *>(def->type());
        return_null_if_null(method);

        if (!method->return_type()->is_compatible(expression->type())) {
            report(TypeMismatchError(expression, def->given_return_type().get()));
            return nullptr;
        }
    } else {
        report(TypeMismatchError(node, nullptr));
        return nullptr;
    }

    expression->copy_type_from(expression);

    return node;
}

ast::Node *TypeInferrer::visit_spawn(ast::Spawn *node) {
    Visitor::visit_spawn(node);
    node->copy_type_from(node->call().get());
    return node;
}

ast::Node *TypeInferrer::visit_case(ast::Case *node) {
    Visitor::visit_case(node);
    node->copy_type_from(node->body().get());
    return node;
}

ast::Node *TypeInferrer::visit_switch(ast::Switch *node) {
    Visitor::visit_switch(node);

    // FIXME make this a union of the typesystem

    node->copy_type_from(node->cases()[0].get());

    return node;
}

ast::Node *TypeInferrer::visit_parameter(ast::Parameter *node) {
    auto symbol = scope()->lookup(this, node, node->name()->value());
    return_null_if_null(symbol);

    if (node->given_type()) {
        visit(node->given_type().get());
        return_null_if_null(node->given_type());

        node->set_type(instance_type(node->given_type().get()));
    } else {
        auto type = new typesystem::ParameterType();
        node->set_type(type->create(this, node));
    }

    return_null_if_null_type(node);

    symbol->copy_type_from(node);

    return node;
}

ast::Node *TypeInferrer::visit_let(ast::Let *node) {
    visit(node->assignment().get());

    if (node->body()) {
        visit(node->body().get());
        node->copy_type_from(node->body());
    } else {
        node->copy_type_from(node->assignment().get());
    }

    return node;
}

ast::Node *TypeInferrer::visit_def(ast::Def *node) {
    auto name = node->name()->field().get();

    auto function_symbol = scope()->lookup(this, name);
    if (!function_symbol->has_type()) {
        function_symbol->set_type(new typesystem::Function());
    }

    push_scope(function_symbol);

    auto symbol = scope()->lookup_by_node(this, node);

    push_scope(symbol);

    for (auto &parameter : name->parameters()) {
        auto parameter_symbol = scope()->lookup(this, parameter.get());
        parameter_symbol->set_type(new typesystem::ParameterType());
        visit(parameter.get());
    }

    std::vector<typesystem::Type *> parameter_types;
    for (auto &parameter : node->parameters()) {
        visit(parameter.get());

        if (!parameter->has_type()) {
            pop_scope();
            pop_scope();
            return nullptr;
        }

        parameter_types.push_back(parameter->type());
    }

    if (!node->builtin()) {
        visit(node->body().get());
    }

    typesystem::Type *return_type = nullptr;
    if (node->builtin() || node->given_return_type()) {
        visit(node->given_return_type().get());
        return_type = instance_type(node->given_return_type().get());
    } else {
        return_type = node->body()->type();
    }

    if (return_type == nullptr) {
        pop_scope();
        pop_scope();
        return nullptr;
    }

    auto method = new typesystem::Method(parameter_types, return_type);

    for (size_t i = 0; i < parameter_types.size(); i++) {
        auto &parameter = node->parameters()[i];
        method->set_parameter_inout(parameter_types[i], parameter->inout());
        method->set_parameter_name(i, parameter->name()->value());
    }

    method->set_is_generic(false);

    auto function_type = static_cast<typesystem::Function *>(function_symbol->type());
    function_type->add_method(method);

    pop_scope();

    scope()->rename(this, symbol, method->mangled_name());

    node->set_type(method);
    symbol->copy_type_from(node);

    m_function_stack.push_back(node);

    m_function_stack.pop_back();

    pop_scope();

    return node;
}

ast::Node *TypeInferrer::visit_type(ast::Type *node) {
    auto symbol = scope()->lookup(this, node, node->name()->value());

    if (node->builtin()) {
        node->set_type(builtin_type_from_name(node->name().get()));
        symbol->copy_type_from(node);
        return node;
    }

    push_scope(symbol);

    std::vector<typesystem::ParameterType *> input_parameters;
    for (auto &t : node->name()->parameters()) {
        auto sym = scope()->lookup(this, t.get());
        sym->set_type(new typesystem::ParameterType());

        visit(t.get());

        auto param = dynamic_cast<typesystem::ParameterType *>(t->type());
        assert(param);

        input_parameters.push_back(param);
    }

    typesystem::Type *type;
    if (node->alias()) {
        visit(node->alias().get());

        auto alias = dynamic_cast<typesystem::TypeType *>(node->alias()->type());
        assert(alias);

        type = new typesystem::AliasType(alias, input_parameters);
    } else {
        std::vector<std::string> field_names;
        std::vector<typesystem::TypeType *> field_types;

        for (auto &name : node->field_names()) {
            field_names.push_back(name->value());
        }

        for (auto &type : node->field_types()) {
            visit(type.get());

            auto type_type = dynamic_cast<typesystem::TypeType *>(type->type());
            assert(type_type);
            field_types.push_back(type_type);
        }

        type = new typesystem::RecordType(input_parameters, field_names, field_types);
    }

    node->set_type(type);
    symbol->copy_type_from(node);

    pop_scope();

    return node;
}

ast::Node *TypeInferrer::visit_module(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name().get());
    return_null_if_null(symbol);

    push_scope(symbol);

    auto module = new typesystem::ModuleType();

    visit(node->body().get());

    node->set_type(module);
    symbol->copy_type_from(node);

    pop_scope();

    return node;
}

ast::Node *TypeInferrer::visit_import(ast::Import *node) {
    visit(node->path().get());
    node->set_type(new typesystem::Void());
    return node;
}

ast::Node *TypeInferrer::visit_source_file(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        visit(import.get());
    }

    visit(node->code().get());

    node->copy_type_from(node->code().get());

    return node;
}
