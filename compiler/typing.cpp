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
#define return_if_null_type(node) return_if_null(node->type())

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

types::Type *Inferrer::instance_type(ast::Name *identifier) {
    return instance_type(identifier, identifier->value(), identifier->parameters());
}

bool Inferrer::infer_call_type_parameters(ast::Call *call, std::vector<types::Type *> parameter_types, std::vector<types::Type *> argument_types) {
    assert(parameter_types.size() == argument_types.size());

    int i = 0;
    for (auto t : parameter_types) {
        auto dt = dynamic_cast<types::Parameter *>(t);
        if (dt) {
            auto it = call->inferred_type_parameters.find(dt->type());
            if (it != call->inferred_type_parameters.end() && !it->second->is_compatible(argument_types[i])) {
                report(TypeMismatchError(call, argument_types[i], it->second));
                return false;
            } else {
                call->inferred_type_parameters[dt->type()] = argument_types[i];
            }
        } else {
            auto dt2 = dynamic_cast<types::ParameterType *>(t);
            if (dt2) {
                auto arg = dynamic_cast<types::TypeType *>(argument_types[i]);
                assert(arg);

                auto it = call->inferred_type_parameters.find(dt2);
                if (it != call->inferred_type_parameters.end() && !it->second->is_compatible(argument_types[i])) {
                    report(TypeMismatchError(call, argument_types[i], it->second));
                    return false;
                } else {
                    call->inferred_type_parameters[dt2] = arg->create(this, call);
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

void Inferrer::visit(ast::Block *block) {
    for (auto statement : block->expressions()) {
        statement->accept(this);
    }

    if (block->empty()) {
        block->set_type(new types::Void());
    } else {
        block->copy_type_from(block->expressions().back());
    }
}

void Inferrer::visit(ast::Name *expression) {
    auto symbol = scope()->lookup(this, expression);
    if (symbol == nullptr) {
        return;
    }

    if (dynamic_cast<types::TypeType *>(symbol->type)) {
        expression->set_type(find_type(expression));
    } else {
        // it *must* be empty
        assert(expression->parameters.empty());
        expression->set_type(symbol->type);
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

void Inferrer::visit(ast::IntegerLiteral *expression) {
    expression->set_type(instance_type(expression, "Int64"));
}

void Inferrer::visit(ast::FloatLiteral *expression) {
    expression->set_type(instance_type(expression, "Float"));
}

void Inferrer::visit(ast::ImaginaryLiteral *expression) {
    expression->set_type(instance_type(expression, "Complex"));
}

void Inferrer::visit(ast::StringLiteral *expression) {
    expression->set_type(instance_type(expression, "String"));
}

void Inferrer::visit(ast::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    std::vector<types::Type *> types;
    for (auto element : sequence->elements) {
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
        auto array_type = find_type(sequence, "Array")->with_parameters(p);
        sequence->set_type(array_type->create(this, sequence));
    } else {
        // FIXME show error
        sequence->set_type(nullptr);
    }
}

void Inferrer::visit(ast::MappingLiteral *mapping) {
    report(TypeInferenceError(mapping));
}

void Inferrer::visit(ast::RecordLiteral *expression) {
    expression->name->accept(this);

    for (auto value : expression->field_values) {
        value->accept(this);
    }

    expression->set_type(instance_type(expression->name));

    // TODO check matches definition of record
}

void Inferrer::visit(ast::TupleLiteral *expression) {
    std::vector<types::Type *> element_types;

    for (auto value : expression->elements()) {
        value->accept(this);
        element_types.push_back(value->type());
    }

    expression->set_type(new types::Tuple(element_types));
}

void Inferrer::visit(ast::Call *expression) {
    expression->operand->accept(this);
    return_if_null_type(expression->operand);

    std::vector<types::Type *> argument_types;
    for (auto arg : expression->arguments) {
        arg->accept(this);
        argument_types.push_back(arg->type());

        if (!arg->has_type()) {
            return;
        }
    }

    types::Function *function = dynamic_cast<types::Function *>(expression->operand->type());
    if (function == nullptr) {
        expression->set_type(new types::Function());
        report(TypeMismatchError(expression->operand, expression));
        delete expression->type();
        expression->set_type(nullptr);
        // FIXME make the construct accept a type directly
        return;
    }

    auto method = function->find_method(expression, argument_types);
    if (method == nullptr) {
        std::stringstream ss;
        for (auto type : argument_types) {
            ss << type->name();
            if (type != argument_types.back()) {
                ss << ", ";
            }
        }

        report(UndefinedError(expression, "Method with " + ss.str() + " types"));
        return;
    }

    if (!infer_call_type_parameters(expression, method->parameter_types(), argument_types)) {
        report(InternalError(expression, "Could not infer type parameters."));
        return;
    }

    auto return_type = replace_type_parameters(method->return_type(),
                                               expression->inferred_type_parameters);

    std::cout << "call " << return_type << std::endl;

    expression->set_type(return_type);
}

void Inferrer::visit(ast::CCall *ccall) {
    for (auto param : ccall->parameters) {
        param->accept(this);
        param->set_type(instance_type(param));
    }

    for (auto arg : ccall->arguments) {
        arg->accept(this);
    }

    // TODO check arg and param types match

    ccall->returnType->accept(this);

    ccall->set_type(instance_type(ccall->returnType));
}

void Inferrer::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    cast->new_type->accept(this);

    cast->set_type(instance_type(cast->new_type));
}

void Inferrer::visit(ast::Assignment *expression) {
    auto symbol = scope()->lookup(this, expression->lhs->name());
    return_if_null(symbol);

    expression->rhs->accept(this);
    return_if_null_type(expression->rhs);

    expression->lhs->accept(this);
    if (!expression->lhs->has_type()) {
        expression->lhs->copy_type_from(expression->rhs);
    }

    return_if_null_type(expression->lhs);

    if (!expression->lhs->has_compatible_type_with(expression->rhs)) {
        report(TypeMismatchError(expression->lhs, expression->rhs));
        return;
    }

    expression->copy_type_from(expression->lhs);
    symbol->copy_type_from(expression);
}

void Inferrer::visit(ast::Selector *expression) {
    expression->operand->accept(this);
    return_if_null_type(expression->operand);

    auto selectable = dynamic_cast<types::Selectable *>(expression->operand->type());
    if (selectable == nullptr) {
        report(TypeMismatchError(expression->operand, expression));
        return;
    }

    auto field_type = selectable->child_type(expression->name->value());
    if (field_type == nullptr) {
        report(UndefinedError(expression->name, expression->name->value()));
        return;
    }

    expression->set_type(field_type);
}

void Inferrer::visit(ast::While *expression) {
    expression->condition()->accept(this);
    expression->body()->accept(this);

    expression->copy_type_from(expression->body());
}

void Inferrer::visit(ast::If *expression) {
    m_in_if = true;
    expression->condition->accept(this);
    m_in_if = false;

    expression->true_case->accept(this);
    if (expression->false_case) {
        expression->false_case->accept(this);
    }

    // FIXME return a union type
    expression->copy_type_from(expression->true_case);
}

void Inferrer::visit(ast::Return *expression) {
    expression->expression->accept(this);
    return_if_null_type(expression->expression)

    if (!m_functionStack.empty()) {
        auto def = m_functionStack.back();
        auto method = static_cast<types::Method *>(def->type());
        return_if_null(method);

        if (!method->return_type()->is_compatible(expression->expression->type())) {
            report(TypeMismatchError(expression->expression, def->returnType));
            return;
        }
    } else {
        report(TypeMismatchError(expression, nullptr));
        return;
    }

    expression->copy_type_from(expression->expression);
}

void Inferrer::visit(ast::Spawn *expression) {
    expression->call->accept(this);
    expression->copy_type_from(expression->call);
}

void Inferrer::visit(ast::Switch *expression) {
    expression->expression()->accept(this);

    for (auto entry : expression->cases()) {
        entry->condition()->accept(this);

        if (entry->assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);

        entry->copy_type_from(entry->body());
    }

    if (expression->default_case()) {
        expression->default_case()->accept(this);
    }

    // TODO ensure all cases are the same type

    expression->copy_type_from(expression->cases()[0]);
}

void Inferrer::visit(ast::Parameter *parameter) {
    auto symbol = scope()->lookup(this, parameter,
                                      parameter->name->value());

    return_if_null(symbol);

    parameter->typeNode->accept(this);

    parameter->set_type(instance_type(parameter->typeNode));
    return_if_null_type(parameter);

    symbol->type = parameter->type();
}

void Inferrer::visit(ast::VariableDefinition *definition) {
    definition->assignment->accept(this);
    definition->body()->accept(this);

    definition->copy_type_from(definition->body());
}

void Inferrer::visit(ast::FunctionDefinition *definition) {
    auto functionSymbol = scope()->lookup(this, definition->name());
    auto function = static_cast<types::Function *>(functionSymbol->type);

    auto symbol = functionSymbol->nameSpace->lookup_by_node(this,
                                                            definition);

    push_scope(symbol);

    for (auto p : definition->name()->parameters()) {
        p->accept(this);
    }

    std::vector<types::Type *> parameterTypes;
    std::vector<std::string> officialParameterOrder;
    for (auto parameter : definition->parameters) {
        parameter->accept(this);

        if (!parameter->has_type()) {
            pop_scope();
            return;
        }

        parameterTypes.push_back(parameter->type());
        officialParameterOrder.push_back(parameter->name->value());
    }

    definition->body->accept(this);

    types::Type *return_type;
    if (definition->returnType) {
        definition->returnType->accept(this);
        return_type = instance_type(definition->returnType);
    } else {
        return_type = definition->body->type();
    }

    if (return_type == nullptr) {
        pop_scope();
        return;
    }

    auto method = new types::Method(parameterTypes, return_type);

    for (size_t i = 0; i < parameterTypes.size(); i++) {
        if (definition->parameters[i]->inout) {
            method->set_parameter_inout(parameterTypes[i], true);
        }
    }

    method->set_is_generic(definition->name()->has_parameters());
    function->add_method(method);

    functionSymbol->nameSpace->rename(this, symbol, method->mangled_name());

    symbol->type = method;
    definition->set_type(method);

    m_functionStack.push_back(definition);

    assert(m_functionStack.back() == definition);
    m_functionStack.pop_back();

    pop_scope();
}

void Inferrer::visit(ast::TypeDefinition *definition) {
    auto symbol = scope()->lookup(this, definition, definition->name()->value());

    push_scope(symbol);

    std::vector<types::ParameterType *> input_parameters;
    for (auto t : definition->name()->parameters()) {
        t->accept(this);

        auto param = dynamic_cast<types::ParameterType *>(t->type());
        assert(param);

        input_parameters.push_back(param);
    }

    types::Type *type;
    if (definition->alias) {
        m_as_type = true;
        definition->alias->accept(this);
        m_as_type = false;

        auto alias = dynamic_cast<types::TypeType *>(definition->alias->type());
        assert(alias);

        type = new types::AliasType(alias, input_parameters);
    } else {
        std::vector<std::string> field_names;
        std::vector<types::TypeType *> field_types;

        for (auto name : definition->field_names) {
            field_names.push_back(name->value());
        }

        for (auto type : definition->field_types) {
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
    definition->set_type(type);

    pop_scope();
}

void Inferrer::visit(ast::ImportExpression *statement) {
    statement->path->accept(this);
    statement->set_type(new types::Void());
}

void Inferrer::visit(ast::SourceFile *module) {
    module->code->accept(this);
    module->copy_type_from(module->code);
}

Checker::Checker(symboltable::Namespace *root_namespace)
{
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
    if (!expression->has_type()) {
        report(InternalError(expression, "No type given for: " + Token::as_string(expression->token().kind)));
    }
}

void Checker::visit(ast::Block *block) {
    for (auto expression : block->expressions()) {
        expression->accept(this);
    }

    check_not_null(block);
}

void Checker::visit(ast::Name *identifier) {
    for (auto p : identifier->parameters()) {
        p->accept(this);
    }

    check_not_null(identifier);
}

void Checker::visit(ast::VariableDeclaration *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::IntegerLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(ast::FloatLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(ast::ImaginaryLiteral *imaginary) {
    check_not_null(imaginary);
}

void Checker::visit(ast::StringLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(ast::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    check_not_null(sequence);
}

void Checker::visit(ast::MappingLiteral *mapping) {
    for (size_t i = 0; i < mapping->keys.size(); i++) {
        mapping->keys[i]->accept(this);
        mapping->values[i]->accept(this);
    }

    check_not_null(mapping);
}

void Checker::visit(ast::RecordLiteral *expression) {
    expression->name->accept(this);

    for (auto value : expression->field_values) {
        value->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(ast::TupleLiteral *expression) {
    for (auto element : expression->elements()) {
        element->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(ast::Call *expression) {
    expression->operand->accept(this);

    for (auto arg : expression->arguments) {
        arg->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(ast::CCall *ccall) {
    // ccall->name->accept(this);

    for (auto param : ccall->parameters) {
        param->accept(this);
    }

    ccall->returnType->accept(this);

    for (auto arg : ccall->arguments) {
        arg->accept(this);
    }

    check_not_null(ccall);
}

void Checker::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    cast->new_type->accept(this);

    check_not_null(cast);
}

void Checker::visit(ast::Assignment *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Selector *expression) {
    expression->operand->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::While *expression) {
    expression->condition()->accept(this);
    expression->body()->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::If *expression) {
    expression->condition->accept(this);
    expression->true_case->accept(this);
    if (expression->false_case) {
        expression->false_case->accept(this);
    }
    check_not_null(expression);
}

void Checker::visit(ast::Return *expression) {
    expression->expression->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Spawn *expression) {
    expression->call->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Switch *expression) {
    expression->expression()->accept(this);

    for (auto entry : expression->cases()) {
        entry->condition()->accept(this);

        if (entry->assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);
        check_not_null(entry);
    }

    if (expression->default_case()) {
        check_not_null(expression->default_case());
    }

    check_not_null(expression);
}

void Checker::visit(ast::Parameter *parameter) {
    parameter->typeNode->accept(this);

    check_not_null(parameter);
}

void Checker::visit(ast::VariableDefinition *definition) {
    check_not_null(definition);

    definition->assignment->accept(this);
    definition->body()->accept(this);

    check_types(definition, definition->body());
}

void Checker::visit(ast::FunctionDefinition *definition) {
    check_not_null(definition);

    auto functionSymbol = scope()->lookup(this, definition->name());

    auto method = static_cast<types::Method *>(definition->type());

    auto symbol = functionSymbol->nameSpace->lookup(this, definition, method->mangled_name());

    push_scope(symbol);

    definition->name()->accept(this);

    for (auto p : definition->name()->parameters()) {
        p->accept(this);
    }

    if (definition->returnType) {
        definition->returnType->accept(this);
    }

    for (auto p : definition->parameters) {
        p->accept(this);
    }

    definition->body->accept(this);

    pop_scope();
}

void Checker::visit(ast::TypeDefinition *definition) {
    check_not_null(definition);

    definition->name()->accept(this);

    auto symbol = scope()->lookup(this, definition->name());

    push_scope(symbol);

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        /*for (auto name : definition->field_names) {
            name->accept(this);
        }*/

        for (auto type : definition->field_types) {
            type->accept(this);
        }
    }

    pop_scope();
}

void Checker::visit(ast::ImportExpression *statement) {
    statement->path->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::SourceFile *module) {
    module->code->accept(this);
    check_not_null(module);
}
