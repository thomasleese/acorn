//
// Created by Thomas Leese on 12/01/2017.
//

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>

#include "ast.h"
#include "diagnostics.h"
#include "types.h"

#include "typing.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::typing;

#define return_if_null(thing) if (thing == nullptr) return;
#define return_null_if_null(thing) if (thing == nullptr) return nullptr;
#define return_if_false(thing) if (thing == false) return;
#define return_if_null_type(node) return_if_false(node->has_type())

Inferrer::Inferrer(symboltable::Namespace *root_namespace)
        : m_in_if(false), m_as_type(false)
{
    push_scope(root_namespace);
}

Inferrer::~Inferrer() {

}

types::TypeType *Inferrer::find_type_constructor(ast::Node *node, std::string name) {
    auto symbol = scope()->lookup(this, node, name);
    return_null_if_null(symbol);
    return dynamic_cast<types::TypeType *>(symbol->type);
}

types::TypeType *Inferrer::find_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters) {
    std::vector<types::Type *> parameterTypes;

    for (auto parameter : parameters) {
        parameter->accept(this);
        parameterTypes.push_back(parameter->type());
    }

    types::TypeType *typeConstructor = find_type_constructor(node, name);
    if (typeConstructor != nullptr) {
        return typeConstructor->with_parameters(parameterTypes);
    } else {
        report(InvalidTypeConstructor(node));
        return nullptr;
    }
}

types::TypeType *Inferrer::find_type(ast::Node *node, std::string name) {
    return find_type(node, name, std::vector<ast::Name *>());
}

types::TypeType *Inferrer::find_type(ast::Name *type) {
    return find_type(type, type->value(), type->parameters());
}

types::Type *Inferrer::instance_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters) {
    types::TypeType *type_constructor = find_type(node, name, parameters);
    if (type_constructor == nullptr) {
        return nullptr;
    }

    return type_constructor->create(this, node);
}

types::Type *Inferrer::instance_type(ast::Node *node, std::string name) {
    return instance_type(node, name, std::vector<ast::Name *>());
}

types::Type *Inferrer::instance_type(ast::Name *name) {
    return instance_type(name, name->value(), name->parameters());
}

types::Type *Inferrer::builtin_type_from_name(ast::Name *node) {
    std::string name = node->value();

    if (name == "Void") {
        return new types::VoidType();
    } else if (name == "Bool") {
        return new types::BooleanType();
    } else if (name == "Int8") {
        return new types::IntegerType(8);
    } else if (name == "Int16") {
        return new types::IntegerType(16);
    } else if (name == "Int32") {
        return new types::IntegerType(32);
    } else if (name == "Int64") {
        return new types::IntegerType(64);
    } else if (name == "Int128") {
        return new types::IntegerType(128);
    } else if (name == "UInt8") {
        return new types::UnsignedIntegerType(8);
    } else if (name == "UInt16") {
        return new types::UnsignedIntegerType(16);
    } else if (name == "UInt32") {
        return new types::UnsignedIntegerType(32);
    } else if (name == "UInt64") {
        return new types::UnsignedIntegerType(64);
    } else if (name == "UInt128") {
        return new types::UnsignedIntegerType(128);
    } else if (name == "Float16") {
        return new types::FloatType(16);
    } else if (name == "Float32") {
        return new types::FloatType(32);
    } else if (name == "Float64") {
        return new types::FloatType(64);
    } else if (name == "Float128") {
        return new types::FloatType(128);
    } else if (name == "UnsafePointer") {
        return new types::UnsafePointerType();
    } else if (name == "Function") {
        return new types::FunctionType();
    } else if (name == "Method") {
        return new types::MethodType();
    } else if (name == "Tuple") {
        return new types::TupleType();
    } else if (name == "Type") {
        return new types::TypeDescriptionType();
    } else {
        report(InternalError(node, "Unknown builtin type."));
        return nullptr;
    }
}

bool Inferrer::infer_call_type_parameters(ast::Call *call, std::vector<types::Type *> parameter_types, std::vector<types::Type *> argument_types) {
    assert(parameter_types.size() == argument_types.size());

    int i = 0;
    for (auto t : parameter_types) {
        auto inferred_type_parameters = call->inferred_type_parameters();

        auto dt = dynamic_cast<types::Parameter *>(t);
        if (dt) {
            auto it = inferred_type_parameters.find(dt->type());
            if (it != inferred_type_parameters.end() && !it->second->is_compatible(argument_types[i])) {
                report(TypeMismatchError(call, argument_types[i], it->second));
                return false;
            } else {
                call->add_inferred_type_parameter(dt->type(), argument_types[i]);
            }
        } else {
            auto dt2 = dynamic_cast<types::ParameterType *>(t);
            if (dt2) {
                auto arg = dynamic_cast<types::TypeType *>(argument_types[i]);

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

types::Type *Inferrer::replace_type_parameters(types::Type *type, std::map<types::ParameterType *, types::Type *> replacements) {
    auto parameter = dynamic_cast<types::Parameter *>(type);
    if (parameter) {
        auto it = replacements.find(parameter->type());
        assert(it != replacements.end());
        type = it->second;
    }

    assert(type);

    auto parameters = type->parameters();

    if (parameters.empty()) {
        return type;
    } else {
        for (size_t i = 0; i < parameters.size(); i++) {
            parameters[i] = replace_type_parameters(parameters[i], replacements);
        }

        return type->with_parameters(parameters);
    }
}

void Inferrer::visit(ast::Block *node) {
    for (auto &expression : node->expressions()) {
        expression->accept(this);
    }

    if (node->empty()) {
        node->set_type(new types::Void());
    } else {
        node->copy_type_from(node->last_expression());
    }
}

void Inferrer::visit(ast::Name *node) {
    auto symbol = scope()->lookup(this, node);
    if (symbol == nullptr) {
        return;
    }

    if (dynamic_cast<types::TypeType *>(symbol->type)) {
        node->set_type(find_type(node));
    } else {
        // it *must* be empty
        assert(node->parameters.empty());
        node->set_type(symbol->type);
    }
}

void Inferrer::visit(ast::VariableDeclaration *node) {
    node->name()->accept(this);

    auto symbol = scope()->lookup(this, node->name());

    if (node->has_given_type()) {
        m_as_type = true;
        node->given_type()->accept(this);
        m_as_type = false;

        node->set_type(instance_type(node->given_type()));
        symbol->type = node->type();
    }
}

void Inferrer::visit(ast::Int *node) {
    node->set_type(instance_type(node, "Int64"));
}

void Inferrer::visit(ast::Float *node) {
    node->set_type(instance_type(node, "Float64"));
}

void Inferrer::visit(ast::Complex *node) {
    node->set_type(instance_type(node, "Complex"));
}

void Inferrer::visit(ast::String *node) {
    node->set_type(instance_type(node, "String"));
}

void Inferrer::visit(ast::List *node) {
    auto elements = node->elements();

    for (auto element : elements) {
        element->accept(this);
    }

    std::vector<types::Type *> types;
    for (auto element : elements) {
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
        std::vector<types::Type *> p;
        p.push_back(types[0]);
        auto array_type = find_type(node, "Array")->with_parameters(p);
        node->set_type(array_type->create(this, node));
    } else {
        // FIXME show error
        node->set_type(nullptr);
    }
}

void Inferrer::visit(ast::Dictionary *node) {
    report(TypeInferenceError(node));
}

void Inferrer::visit(ast::Tuple *node) {
    std::vector<types::Type *> element_types;

    for (auto element : node->elements()) {
        element->accept(this);
        element_types.push_back(element->type());
    }

    node->set_type(new types::Tuple(element_types));
}

void Inferrer::visit(ast::Call *node) {
    node->operand()->accept(this);
    return_if_null_type(node->operand());

    for (auto &argument : node->positional_arguments()) {
        argument->accept(this);
        return_if_null_type(argument);
    }

    for (auto &entry : node->keyword_arguments()) {
        entry.second->accept(this);
        return_if_null_type(entry.second);
    }

    auto function = dynamic_cast<types::Function *>(node->operand_type());
    if (function == nullptr) {
        node->set_type(new types::Function());
        report(TypeMismatchError(node->operand(), node));
        delete node->type();
        node->set_type(nullptr);
        // FIXME make the construct accept a type directly
        return;
    }

    auto method = function->find_method(node);
    if (method == nullptr) {
        report(UndefinedError(node, "Method for these types not available."));
        return;
    }

    node->set_method_index(function->index_of(method));

    if (!infer_call_type_parameters(node, method->parameter_types(), method->ordered_argument_types(node))) {
        report(InternalError(node, "Could not infer type parameters."));
        return;
    }

    auto return_type = replace_type_parameters(method->return_type(),
                                               node->inferred_type_parameters());

    if (method->is_generic()) {
        node->set_method_specialisation_index(method->no_generic_specialisation());
        method->add_generic_specialisation(node->inferred_type_parameters());
    }

    node->set_type(return_type);
}

void Inferrer::visit(ast::CCall *node) {
    for (auto param : node->parameters()) {
        param->accept(this);
        param->set_type(instance_type(param));
    }

    for (auto argument : node->arguments()) {
        argument->accept(this);
    }

    // TODO check arg and param types match

    node->given_return_type()->accept(this);

    node->set_type(instance_type(node->given_return_type()));
}

void Inferrer::visit(ast::Cast *node) {
    node->operand()->accept(this);
    node->new_type()->accept(this);

    node->set_type(instance_type(node->new_type()));
}

void Inferrer::visit(ast::Assignment *node) {
    auto symbol = scope()->lookup(this, node->lhs()->name());
    return_if_null(symbol);

    auto rhs = node->rhs();

    if (!node->builtin()) {
        rhs->accept(this);
        return_if_null_type(rhs);
    }

    auto lhs = node->lhs();

    lhs->accept(this);
    if (!lhs->has_type()) {
        lhs->copy_type_from(node->rhs());
    }

    return_if_null_type(lhs);

    if (!node->builtin()) {
        if (!lhs->has_compatible_type_with(rhs)) {
            report(TypeMismatchError(lhs, rhs));
            return;
        }
    }

    node->copy_type_from(lhs);
    symbol->copy_type_from(node);
}

void Inferrer::visit(ast::Selector *node) {
    auto operand = node->operand();

    operand->accept(this);
    return_if_null_type(operand);

    auto module = dynamic_cast<types::Module *>(operand->type());
    auto record_type = dynamic_cast<types::RecordType *>(operand->type());
    auto record = dynamic_cast<types::Record *>(operand->type());

    if (module) {
        auto module_name = static_cast<ast::Name *>(operand);

        auto symbol = scope()->lookup(this, module_name);
        return_if_null(symbol);

        auto child_symbol = symbol->nameSpace->lookup(this, node->field());
        return_if_null(child_symbol);

        node->set_type(child_symbol->type);
    } else if (record_type) {
        if (node->field()->value() == "new") {
            node->set_type(record_type->constructor());
        } else {
            report(UndefinedError(node->field(), "new"));
        }
    } else if (record) {
        auto field = node->field();

        auto field_type = record->child_type(field->value());
        if (field_type != nullptr) {
            node->set_type(field_type);
        } else {
            report(UndefinedError(field));
        }
    } else {
        report(TypeMismatchError(operand, operand->type()->name(), "module, record type or record"));
    }
}

void Inferrer::visit(ast::While *node) {
    node->condition()->accept(this);
    node->body()->accept(this);

    node->copy_type_from(node->body());
}

void Inferrer::visit(ast::If *node) {
    m_in_if = true;
    node->condition()->accept(this);
    m_in_if = false;

    node->true_case()->accept(this);
    if (node->has_false_case()) {
        node->false_case()->accept(this);
    }

    // FIXME return a union type
    node->copy_type_from(node->true_case());
}

void Inferrer::visit(ast::Return *node) {
    auto expression = node->expression();

    expression->accept(this);
    return_if_null_type(expression)

    if (!m_functionStack.empty()) {
        auto def = m_functionStack.back();
        auto method = static_cast<types::Method *>(def->type());
        return_if_null(method);

        if (!method->return_type()->is_compatible(expression->type())) {
            report(TypeMismatchError(expression, def->given_return_type()));
            return;
        }
    } else {
        report(TypeMismatchError(node, nullptr));
        return;
    }

    expression->copy_type_from(expression);
}

void Inferrer::visit(ast::Spawn *node) {
    auto call = node->call();
    call->accept(this);
    node->copy_type_from(call);
}

void Inferrer::visit(ast::Switch *node) {
    node->expression()->accept(this);

    for (auto entry : node->cases()) {
        entry->condition()->accept(this);

        if (entry->has_assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);

        entry->copy_type_from(entry->body());
    }

    if (node->has_default_case()) {
        node->default_case()->accept(this);
    }

    // FIXME make this a union of the types

    node->copy_type_from(node->cases()[0]);
}

void Inferrer::visit(ast::Parameter *node) {
    auto symbol = scope()->lookup(this, node, node->name()->value());
    return_if_null(symbol);

    node->given_type()->accept(this);
    return_if_null(node->given_type());

    node->set_type(instance_type(node->given_type()));
    return_if_null_type(node);

    symbol->type = node->type();
}

void Inferrer::visit(ast::Let *node) {
    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
        node->copy_type_from(node->body());
    } else {
        node->copy_type_from(node->assignment());
    }
}

void Inferrer::visit(ast::Def *node) {
    auto name = static_cast<ast::Name *>(node->name());

    auto function_symbol = scope()->lookup(this, name);
    if (function_symbol->type == nullptr) {
        function_symbol->type = new types::Function();
    }

    push_scope(function_symbol);

    auto symbol = scope()->lookup_by_node(this, node);

    push_scope(symbol);

    for (auto parameter : name->parameters()) {
        auto parameter_symbol = scope()->lookup(this, parameter);
        parameter_symbol->type = new types::ParameterType();
        parameter->accept(this);
    }

    std::vector<types::Type *> parameter_types;
    for (auto parameter : node->parameters()) {
        parameter->accept(this);

        if (!parameter->has_type()) {
            pop_scope();
            pop_scope();
            return;
        }

        parameter_types.push_back(parameter->type());
    }

    if (!node->builtin()) {
        node->body()->accept(this);
    }

    types::Type *return_type = nullptr;
    if (node->builtin() || node->has_given_return_type()) {
        node->given_return_type()->accept(this);
        return_type = instance_type(node->given_return_type());
    } else {
        return_type = node->body()->type();
    }

    if (return_type == nullptr) {
        pop_scope();
        pop_scope();
        return;
    }

    auto method = new types::Method(parameter_types, return_type);

    for (size_t i = 0; i < parameter_types.size(); i++) {
        auto parameter = node->parameter(i);
        method->set_parameter_inout(parameter_types[i], parameter->inout());
        method->set_parameter_name(i, parameter->name()->value());
    }

    method->set_is_generic(name->has_parameters());

    auto function_type = static_cast<types::Function *>(function_symbol->type);
    function_type->add_method(method);

    pop_scope();

    scope()->rename(this, symbol, method->mangled_name());

    node->set_type(method);
    symbol->copy_type_from(node);

    m_functionStack.push_back(node);

    m_functionStack.pop_back();

    pop_scope();
}

void Inferrer::visit(ast::Type *node) {
    auto symbol = scope()->lookup(this, node, node->name()->value());

    if (node->builtin()) {
        node->set_type(builtin_type_from_name(node->name()));
        symbol->copy_type_from(node);
        return;
    }

    push_scope(symbol);

    std::vector<types::ParameterType *> input_parameters;
    for (auto t : node->name()->parameters()) {
        auto sym = scope()->lookup(this, t);
        sym->type = new types::ParameterType();

        t->accept(this);

        auto param = dynamic_cast<types::ParameterType *>(t->type());
        assert(param);

        input_parameters.push_back(param);
    }

    types::Type *type;
    if (node->has_alias()) {
        m_as_type = true;
        node->alias()->accept(this);
        m_as_type = false;

        auto alias = dynamic_cast<types::TypeType *>(node->alias()->type());
        assert(alias);

        type = new types::AliasType(alias, input_parameters);
    } else {
        std::vector<std::string> field_names;
        std::vector<types::TypeType *> field_types;

        for (auto name : node->field_names()) {
            field_names.push_back(name->value());
        }

        for (auto type : node->field_types()) {
            m_as_type = true;
            type->accept(this);
            m_as_type = false;

            auto type_type = dynamic_cast<types::TypeType *>(type->type());
            assert(type_type);
            field_types.push_back(type_type);
        }

        type = new types::RecordType(input_parameters, field_names, field_types);
    }

    symbol->type = type;
    node->set_type(type);

    pop_scope();
}

void Inferrer::visit(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name());
    return_if_null(symbol);

    push_scope(symbol);

    node->body()->accept(this);
    node->set_type(new types::Module());
    symbol->copy_type_from(node);

    pop_scope();
}

void Inferrer::visit(ast::Import *node) {
    node->path()->accept(this);
    node->set_type(new types::Void());
}

void Inferrer::visit(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);

    node->copy_type_from(node->code());
}

Checker::Checker(symboltable::Namespace *root_namespace) {
    push_scope(root_namespace);
}

Checker::~Checker() {

}

void Checker::check_types(ast::Expression *lhs, ast::Expression *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    if (!lhs->has_compatible_type_with(rhs)) {
        report(TypeMismatchError(rhs, lhs));
    }
}

void Checker::check_not_null(ast::Expression *expression) {
    check_not_null(*expression);
}

void Checker::check_not_null(ast::Expression &expression) {
    if (!expression.has_type()) {
        report(InternalError(expression, "No type given for: " + Token::as_string(expression.token().kind)));
    }
}

void Checker::visit(ast::Block *node) {
    for (auto &expression : node->expressions()) {
        expression->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Name *node) {
    for (auto &parameter : node->parameters()) {
        parameter->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::VariableDeclaration *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Int *node) {
    check_not_null(node);
}

void Checker::visit(ast::Float *node) {
    check_not_null(node);
}

void Checker::visit(ast::Complex *node) {
    check_not_null(node);
}

void Checker::visit(ast::String *node) {
    check_not_null(node);
}

void Checker::visit(ast::List *node) {
    for (auto element : node->elements()) {
        element->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Dictionary *node) {
    for (size_t i = 0; i < node->elements_size(); i++) {
        node->key(i)->accept(this);
        node->value(i)->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Tuple *node) {
    for (auto element : node->elements()) {
        element->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Call *node) {
    node->operand()->accept(this);

    for (auto &argument : node->positional_arguments()) {
        argument->accept(this);
    }

    for (auto &entry : node->keyword_arguments()) {
        entry.second->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::CCall *node) {
    // node->name->accept(this);

    for (auto param : node->parameters()) {
        param->accept(this);
    }

    node->given_return_type()->accept(this);

    for (auto argument : node->arguments()) {
        argument->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Cast *node) {
    node->operand()->accept(this);
    node->new_type()->accept(this);

    check_not_null(node);
}

void Checker::visit(ast::Assignment *node) {
    node->lhs()->accept(this);

    if (!node->builtin()) {
        node->rhs()->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Selector *node) {
    node->operand()->accept(this);
    check_not_null(node);
}

void Checker::visit(ast::While *node) {
    node->condition()->accept(this);
    node->body()->accept(this);
    check_not_null(node);
}

void Checker::visit(ast::If *node) {
    node->condition()->accept(this);

    node->true_case()->accept(this);

    if (node->has_false_case()) {
        node->false_case()->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::Return *node) {
    node->expression()->accept(this);
    check_not_null(node);
}

void Checker::visit(ast::Spawn *node) {
    node->call()->accept(this);
    check_not_null(node);
}

void Checker::visit(ast::Switch *node) {
    node->expression()->accept(this);

    for (auto entry : node->cases()) {
        entry->condition()->accept(this);

        if (entry->has_assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);
        check_not_null(entry);
    }

    if (node->has_default_case()) {
        check_not_null(node->default_case());
    }

    check_not_null(node);
}

void Checker::visit(ast::Parameter *node) {
    node->given_type()->accept(this);
    check_not_null(node);
}

void Checker::visit(ast::Let *node) {
    check_not_null(node);

    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
        check_types(node, node->body());
    } else {
        check_types(node, node->assignment());
    }
}

void Checker::visit(ast::Def *node) {
    check_not_null(node);

    auto name = static_cast<ast::Name *>(node->name());

    auto function_symbol = scope()->lookup(this, name);

    push_scope(function_symbol);

    auto method = static_cast<types::Method *>(node->type());

    auto symbol = scope()->lookup(this, node, method->mangled_name());

    push_scope(symbol);

    node->name()->accept(this);

    for (auto p : name->parameters()) {
        p->accept(this);
    }

    if (node->builtin() || node->has_given_return_type()) {
        node->given_return_type()->accept(this);
    }

    for (auto p : node->parameters()) {
        p->accept(this);
    }

    if (!node->builtin()) {
        node->body()->accept(this);
    }

    pop_scope();
    pop_scope();
}

void Checker::visit(ast::Type *node) {
    check_not_null(node);

    node->name()->accept(this);

    auto symbol = scope()->lookup(this, node->name());

    push_scope(symbol);

    if (node->has_alias()) {
        node->alias()->accept(this);
    } else {
        /*for (auto name : node->field_names) {
            name->accept(this);
        }*/

        for (auto type : node->field_types()) {
            type->accept(this);
        }
    }

    pop_scope();
}

void Checker::visit(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name());
    return_if_null(symbol);

    push_scope(symbol);

    node->body()->accept(this);
    check_not_null(node);

    pop_scope();
}

void Checker::visit(ast::Import *node) {
    node->path()->accept(this);
    check_not_null(node);
}

void Checker::visit(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);

    check_not_null(node);
}
