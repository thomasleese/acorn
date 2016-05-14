//
// Created by Thomas Leese on 21/03/2016.
//

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>

#include "ast/nodes.h"
#include "errors.h"
#include "symboltable.h"
#include "types.h"

#include "typing.h"

using namespace acorn;
using namespace acorn::typing;

#define return_if_null(thing) if (thing == nullptr) return;
#define return_if_null_type(node) return_if_null(node->type)

Inferrer::Inferrer(symboltable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Inferrer::~Inferrer() {

}

types::Constructor *Inferrer::find_type_constructor(ast::Node *node, std::string name) {
    auto symbol = m_namespace->lookup(this, node, name);
    if (symbol == nullptr) {
        return nullptr;
    }

    types::Constructor *typeConstructor = dynamic_cast<types::Constructor *>(symbol->type);
    if (typeConstructor) {
        return typeConstructor;
    } else {
        return nullptr;
    }
}

types::Type *Inferrer::find_type(ast::Node *node, std::string name, std::vector<ast::Identifier *> parameters) {
    std::vector<types::Type *> parameterTypes;

    for (auto parameter : parameters) {
        parameter->accept(this);
        parameterTypes.push_back(parameter->type);
    }

    types::Constructor *typeConstructor = find_type_constructor(node, name);
    if (typeConstructor != nullptr) {
        return typeConstructor->create(this, node, parameterTypes);
    } else {
        push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

types::Type *Inferrer::find_type(ast::Node *node, std::string name) {
    return find_type(node, name, std::vector<ast::Identifier *>());
}

types::Type *Inferrer::find_type(ast::Identifier *type) {
    return find_type(type, type->value, type->parameters);
}

types::Type *Inferrer::instance_type(ast::Identifier *identifier) {
    types::Constructor *type_constructor = dynamic_cast<types::Constructor *>(identifier->type);
    if (type_constructor) {
        std::vector<types::Type *> parameterTypes;

        for (auto parameter : identifier->parameters) {
            parameter->type = instance_type(parameter);
            parameterTypes.push_back(parameter->type);
        }

        return type_constructor->create(this, identifier, parameterTypes);
    } else {
        push_error(new errors::InvalidTypeConstructor(identifier));
        return nullptr;
    }
}

void Inferrer::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }

    if (block->statements.empty()) {
        block->type = new types::Void();
    } else {
        block->type = block->statements.back()->type;
    }
}

void Inferrer::visit(ast::Identifier *expression) {
    for (auto p : expression->parameters) {
        p->accept(this);
    }

    auto symbol = m_namespace->lookup(this, expression, expression->value);
    if (symbol == nullptr) {
        return;
    }

    expression->type = symbol->type;
}

void Inferrer::visit(ast::BooleanLiteral *expression) {
    expression->type = find_type(expression, "Boolean");
}

void Inferrer::visit(ast::IntegerLiteral *expression) {
    expression->type = find_type(expression, "Integer");
}

void Inferrer::visit(ast::FloatLiteral *expression) {
    expression->type = find_type(expression, "Float");
}

void Inferrer::visit(ast::ImaginaryLiteral *expression) {
    expression->type = find_type(expression, "Complex");
}

void Inferrer::visit(ast::StringLiteral *expression) {
    expression->type = find_type(expression, "String");
}

void Inferrer::visit(ast::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    std::set<types::Type *> types;
    for (auto element : sequence->elements) {
        bool inList = false;
        for (auto type : types) {
            if (type->isCompatible(element->type)) {
                inList = true;
                break;
            }
        }

        if (!inList) {
            types.insert(element->type);
        }
    }

    types::Type *elementType;
    if (types.empty()) {
        elementType = new types::Any();
    } else if (types.size() != 1) {
        elementType = new types::Union(this, sequence, types);
    } else {
        elementType = *(types.begin());
    }

    std::vector<types::Type *> args;
    args.push_back(elementType);
    sequence->type = find_type_constructor(sequence, "Array")->create(this, sequence, args);
}

void Inferrer::visit(ast::MappingLiteral *mapping) {
    push_error(new errors::TypeInferenceError(mapping));
}

void Inferrer::visit(ast::RecordLiteral *expression) {
    expression->name->accept(this);

    for (auto value : expression->field_values) {
        value->accept(this);
    }

    expression->type = instance_type(expression->name);

    // TODO check matches definition of record
}

void Inferrer::visit(ast::Call *expression) {
    expression->operand->accept(this);
    return_if_null_type(expression->operand);

    std::vector<types::Type *> argument_types;
    for (auto arg : expression->arguments) {
        arg->accept(this);
        argument_types.push_back(arg->type);

        if (arg->type == nullptr) {
            return;
        }
    }

    types::Function *function = dynamic_cast<types::Function *>(expression->operand->type);
    if (function == nullptr) {
        expression->type = new types::Function();
        push_error(new errors::TypeMismatchError(expression->operand, expression));
        delete expression->type;
        expression->type = nullptr;
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

        push_error(new errors::UndefinedError(expression, "Method with " + ss.str() + " types"));
    } else {
        expression->type = method->return_type();
    }
}

void Inferrer::visit(ast::CCall *ccall) {
    for (auto param : ccall->parameters) {
        param->accept(this);
        param->type = instance_type(param);
    }

    for (auto arg : ccall->arguments) {
        arg->accept(this);
    }

    // TODO check arg and param types match

    ccall->returnType->accept(this);

    ccall->type = instance_type(ccall->returnType);
}

void Inferrer::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    cast->new_type->accept(this);

    cast->type = instance_type(cast->new_type);
}

void Inferrer::visit(ast::Assignment *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);

    return_if_null_type(expression->lhs)
    return_if_null_type(expression->rhs)

    if (!expression->lhs->type->isCompatible(expression->rhs->type)) {
        push_error(new errors::TypeMismatchError(expression->rhs, expression->lhs));
    } else {
        expression->type = expression->lhs->type;
    }
}

void Inferrer::visit(ast::Selector *expression) {
    expression->operand->accept(this);

    auto record_type = dynamic_cast<types::Record *>(expression->operand->type);
    if (record_type == nullptr) {
        push_error(new errors::TypeMismatchError(expression, expression->operand));
        return;
    }

    auto field_type = record_type->get_field_type(expression->name->value);
    if (field_type == nullptr) {
        push_error(new errors::UndefinedError(expression->name, expression->name->value));
        return;
    }

    expression->type = field_type;
}

void Inferrer::visit(ast::Comma *expression) {
    push_error(new errors::TypeInferenceError(expression));
}

void Inferrer::visit(ast::While *expression) {
    expression->condition->accept(this);
    expression->code->accept(this);

    expression->type = expression->code->type;
}

void Inferrer::visit(ast::For *expression) {
    push_error(new errors::InternalError(expression, "For should never be in the lowered AST."));
}

void Inferrer::visit(ast::If *expression) {
    expression->condition->accept(this);
    expression->trueCode->accept(this);
    if (expression->falseCode) {
        expression->falseCode->accept(this);
    }

    expression->type = expression->trueCode->type;
}

void Inferrer::visit(ast::Return *expression) {
    expression->expression->accept(this);
    return_if_null_type(expression->expression)

    if (m_functionStack.back()) {
        ast::FunctionDefinition *def = m_functionStack.back();
        return_if_null_type(def->returnType)
        if (!def->returnType->type->isCompatible(expression->expression->type)) {
            push_error(new errors::TypeMismatchError(expression->expression, def->returnType));
            return;
        }
    } else {
        push_error(new errors::TypeMismatchError(expression, nullptr));
        return;
    }

    expression->type = expression->expression->type;
}

void Inferrer::visit(ast::Spawn *expression) {
    expression->call->accept(this);
    expression->type = expression->call->type;
}

void Inferrer::visit(ast::Sizeof *expression) {
    expression->identifier->accept(this);
    expression->type = find_type(expression, "Integer64");
}

void Inferrer::visit(ast::Strideof *expression) {
    expression->identifier->accept(this);
    expression->type = find_type(expression, "Integer64");
}

void Inferrer::visit(ast::Parameter *parameter) {
    auto symbol = m_namespace->lookup(this, parameter, parameter->name->value);
    if (symbol == nullptr) {
        return;
    }

    parameter->typeNode->accept(this);

    parameter->type = instance_type(parameter->typeNode);
    return_if_null_type(parameter);

    /*if (parameter->inout) {
        parameter->type = new types::InOut(parameter->type);
    }*/

    symbol->type = parameter->type;
}

void Inferrer::visit(ast::VariableDefinition *definition) {
    auto symbol = m_namespace->lookup(this, definition, definition->name->value);
    if (symbol == nullptr) {
        return;
    }

    types::Type *type = nullptr;

    definition->expression->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
        type = definition->typeNode->type;
    } else {
        type = definition->expression->type;
    }

    if (!type) {
        push_error(new errors::TypeInferenceError(definition));
    }

    symbol->type = type;
    definition->type = type;
}

void Inferrer::visit(ast::FunctionDefinition *definition) {
    auto functionSymbol = m_namespace->lookup(this, definition->name);
    types::Function *function = static_cast<types::Function *>(functionSymbol->type);

    auto symbol = functionSymbol->nameSpace->lookup_by_node(this, definition);

    symboltable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    for (auto p : definition->name->parameters) {
        p->accept(this);
    }

    std::vector<types::Type *> parameterTypes;
    std::vector<std::string> officialParameterOrder;
    for (auto parameter : definition->parameters) {
        parameter->accept(this);

        if (parameter->type == nullptr) {
            m_namespace = oldNamespace;
            return;
        }

        parameterTypes.push_back(parameter->type);
        officialParameterOrder.push_back(parameter->name->value);
    }

    definition->returnType->accept(this);
    return_if_null_type(definition->returnType);
    definition->returnType->type = instance_type(definition->returnType);

    auto method = new types::Method(parameterTypes, definition->returnType->type);

    for (int i = 0; i < parameterTypes.size(); i++) {
        if (definition->parameters[i]->inout) {
            method->set_parameter_inout(parameterTypes[i], true);
        }
    }

    method->set_is_generic(!definition->name->parameters.empty());
    function->add_method(method);

    functionSymbol->nameSpace->rename(this, symbol, method->mangled_name());

    symbol->type = method;
    definition->type = method;

    m_functionStack.push_back(definition);

    definition->code->accept(this);

    assert(m_functionStack.back() == definition);
    m_functionStack.pop_back();

    m_namespace = oldNamespace;
}

void Inferrer::visit(ast::TypeDefinition *definition) {
    auto symbol = m_namespace->lookup(this, definition, definition->name->value);

    symboltable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    std::vector<types::Parameter *> input_parameters;
    for (auto t : definition->name->parameters) {
        t->accept(this);

        auto param = dynamic_cast<types::Parameter *>(t->type);
        if (param == nullptr) {
            push_error(new errors::InvalidTypeParameters(t, 0, 0));
            return;
        }

        input_parameters.push_back(param);
    }

    types::Type *type;
    if (definition->alias) {
        std::vector<types::Type *> outputParameters;
        for (auto t : definition->alias->parameters) {
            t->accept(this);
            outputParameters.push_back(t->type);
        }

        auto type_constructor = find_type_constructor(definition, definition->alias->value);
        definition->alias->type = type_constructor;
        type = new types::AliasConstructor(definition, type_constructor,
                                           input_parameters, outputParameters);
    } else {
        std::vector<std::string> field_names;
        std::vector<types::Constructor *> field_types;
        std::vector<std::vector<types::Type *> > field_parameters;

        for (auto name : definition->field_names) {
            field_names.push_back(name->value);
        }

        for (auto type : definition->field_types) {
            auto type_constructor = find_type_constructor(type, type->value);

            std::vector<types::Type *> parameters;
            for (auto t : type->parameters) {
                t->accept(this);
                parameters.push_back(t->type);
            }

            field_types.push_back(type_constructor);
            field_parameters.push_back(parameters);

            type->type = type_constructor;
        }

        type = new types::RecordConstructor(input_parameters,
                                            field_names, field_types,
                                            field_parameters);
    }

    symbol->type = type;
    definition->type = type;

    m_namespace = oldNamespace;
}

void Inferrer::visit(ast::DefinitionStatement *statement) {
    statement->definition->accept(this);
    statement->type = statement->definition->type;
}

void Inferrer::visit(ast::ExpressionStatement *statement) {
    statement->expression->accept(this);
    statement->type = statement->expression->type;
}

void Inferrer::visit(ast::ImportStatement *statement) {
    statement->path->accept(this);
    statement->type = new types::Void();
}

void Inferrer::visit(ast::SourceFile *module) {
    module->code->accept(this);
    module->type = module->code->type;
}

Checker::Checker(symboltable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Checker::~Checker() {

}

void Checker::check_types(ast::Node *lhs, ast::Node *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    bool compatible = lhs->type->isCompatible(rhs->type);
    if (!compatible) {
        push_error(new errors::TypeMismatchError(lhs, rhs));
    }
}

void Checker::check_not_null(ast::Node *node) {
    if (!node->type) {
        push_error(new errors::InternalError(node, "No type given for: " + Token::rule_string(node->token->rule)));
    }
}

void Checker::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }

    check_not_null(block);
}

void Checker::visit(ast::Identifier *identifier) {
    for (auto p : identifier->parameters) {
        p->accept(this);
    }

    check_not_null(identifier);
}

void Checker::visit(ast::BooleanLiteral *boolean) {
    check_not_null(boolean);
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
    for (int i = 0; i < mapping->keys.size(); i++) {
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

    //symboltable::Symbol *symbol = m_namespace->lookup(expression->lhs);
    //push_error(new errors::ConstantAssignmentError(expression->lhs));
}

void Checker::visit(ast::Selector *expression) {
    expression->operand->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Comma *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::While *expression) {
    expression->condition->accept(this);
    expression->code->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::For *expression) {
    push_error(new errors::InternalAstError(expression));
}

void Checker::visit(ast::If *expression) {
    expression->condition->accept(this);
    expression->trueCode->accept(this);
    if (expression->falseCode) {
        expression->falseCode->accept(this);
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

void Checker::visit(ast::Sizeof *expression) {
    expression->identifier->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Strideof *expression) {
    expression->identifier->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Parameter *parameter) {
    parameter->typeNode->accept(this);

    check_not_null(parameter);
}

void Checker::visit(ast::VariableDefinition *definition) {
    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
    }

    definition->expression->accept(this);

    check_types(definition, definition->expression);
}

void Checker::visit(ast::FunctionDefinition *definition) {
    check_not_null(definition);

    auto functionSymbol = m_namespace->lookup(this, definition->name);

    types::Method *method = static_cast<types::Method *>(definition->type);

    auto symbol = functionSymbol->nameSpace->lookup(this, definition, method->mangled_name());

    symboltable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

    for (auto p : definition->name->parameters) {
        p->accept(this);
    }

    definition->returnType->accept(this);

    for (auto p : definition->parameters) {
        p->accept(this);
    }

    definition->code->accept(this);

    m_namespace = oldNamespace;
}

void Checker::visit(ast::TypeDefinition *definition) {
    // it's valid for the name not to have a type, since it's doesn't exist
    //definition->name->accept(this);

    auto symbol = m_namespace->lookup(this, definition->name);

    symboltable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

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

    check_not_null(definition);

    m_namespace = oldNamespace;
}

void Checker::visit(ast::DefinitionStatement *statement) {
    statement->definition->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::ExpressionStatement *statement) {
    statement->expression->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::ImportStatement *statement) {
    statement->path->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::SourceFile *module) {
    module->code->accept(this);
    check_not_null(module);
}
