//
// Created by Thomas Leese on 21/03/2016.
//

#include <iostream>
#include <set>
#include <sstream>

#include "ast/nodes.h"
#include "Typing.h"
#include "Errors.h"
#include "Types.h"

#include "SymbolTable.h"

using namespace jet;
using namespace Typing;

Inferrer::Inferrer(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Inferrer::~Inferrer() {

}

Types::Constructor *Inferrer::find_type_constructor(ast::Node *node, std::string name) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(node, name);
    Types::Constructor *typeConstructor = dynamic_cast<Types::Constructor *>(symbol->type);
    if (typeConstructor) {
        return typeConstructor;
    } else {
        return nullptr;
    }
}

Types::Type *Inferrer::find_type(ast::Node *node, std::string name, std::vector<ast::Identifier *> parameters) {
    std::vector<Types::Type *> parameterTypes;

    for (auto parameter : parameters) {
        parameter->accept(this);
        parameterTypes.push_back(parameter->type);
    }

    Types::Constructor *typeConstructor = find_type_constructor(node, name);
    if (typeConstructor != nullptr) {
        return typeConstructor->create(node, parameterTypes);
    } else {
        throw errors::InvalidTypeConstructor(node);
    }
}

Types::Type *Inferrer::find_type(ast::Node *node, std::string name) {
    return find_type(node, name, std::vector<ast::Identifier *>());
}

Types::Type *Inferrer::find_type(ast::Identifier *type) {
    return find_type(type, type->value, type->parameters);
}

Types::Type *Inferrer::instance_type(ast::Identifier *identifier) {
    Types::Constructor *type_constructor = dynamic_cast<Types::Constructor *>(identifier->type);
    if (type_constructor) {
        std::vector<Types::Type *> parameterTypes;

        for (auto parameter : identifier->parameters) {
            parameter->type = instance_type(parameter);
            parameterTypes.push_back(parameter->type);
        }

        return type_constructor->create(identifier, parameterTypes);
    } else {
        throw errors::InvalidTypeConstructor(identifier);
    }
}

void Inferrer::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }

    if (block->statements.empty()) {
        block->type = new Types::Void();
    } else {
        block->type = block->statements.back()->type;
    }
}

void Inferrer::visit(ast::Identifier *expression) {
    for (auto p : expression->parameters) {
        p->accept(this);
    }

    SymbolTable::Symbol *symbol = m_namespace->lookup(expression, expression->value);
    expression->type = symbol->type;

    if (!symbol->type) {
        throw errors::UndefinedError(expression, expression->value);
    }
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

    std::set<Types::Type *> types;
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

    Types::Type *elementType;
    if (types.empty()) {
        elementType = new Types::Any();
    } else if (types.size() != 1) {
        elementType = new Types::Union(sequence, types);
    } else {
        elementType = *(types.begin());
    }

    //sequence->type = fin
}

void Inferrer::visit(ast::MappingLiteral *mapping) {
    throw errors::TypeInferenceError(mapping);
}

void Inferrer::visit(ast::RecordLiteral *expression) {
    expression->name->accept(this);

    for (auto value : expression->field_values) {
        value->accept(this);
    }

    expression->type = instance_type(expression->name);

    // TODO check matches definition of record
}

void Inferrer::visit(ast::Argument *argument) {
    argument->value->accept(this);
    if (argument->name) {
        argument->name->type = argument->value->type;
    }
    argument->type = argument->value->type;
}

void Inferrer::visit(ast::Call *expression) {
    expression->operand->accept(this);

    for (auto arg : expression->arguments) {
        arg->accept(this);
    }

    Types::Function *function = dynamic_cast<Types::Function *>(expression->operand->type);
    Types::RecordConstructor *record = dynamic_cast<Types::RecordConstructor *>(expression->operand->type);
    if (function == nullptr && record == nullptr) {
        expression->type = new Types::Union(new Types::Function(), new Types::RecordConstructor());
        throw errors::TypeMismatchError(expression->operand, expression);
    }

    if (record == nullptr) {
        Types::Method *method = function->find_method(expression, expression->arguments);
        expression->type = method->return_type();
    } else {
        expression->type = record->create(expression, std::vector<Types::Type *>());
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
    expression->type = expression->rhs->type;
}

void Inferrer::visit(ast::Selector *expression) {
    expression->operand->accept(this);

    auto recordType = dynamic_cast<Types::Record *>(expression->operand->type);
    if (!recordType) {
        throw errors::TypeInferenceError(expression);
    }

    auto fieldType = recordType->get_field_type(expression->name->value);
    if (!fieldType) {
        throw errors::TypeInferenceError(expression);
    }

    expression->type = fieldType;
}

void Inferrer::visit(ast::Index *expression) {
    /*expression->operand->accept(this);
    expression->index->accept(this);

    auto arrayType = dynamic_cast<Types::Array *>(expression->operand->type);
    if (!arrayType) {
        throw errors::TypeInferenceError(expression);
    }

    auto indexType = dynamic_cast<Types::Integer *>(expression->index->type);
    if (!indexType) {
        throw errors::TypeMismatchError(expression->operand, expression->index);
    }

    expression->type = arrayType->element_type();*/
}

void Inferrer::visit(ast::Comma *expression) {
    throw errors::TypeInferenceError(expression);
}

void Inferrer::visit(ast::While *expression) {
    expression->condition->accept(this);
    expression->code->accept(this);

    expression->type = expression->code->type;
}

void Inferrer::visit(ast::For *expression) {
    throw errors::InternalError(expression, "For should never be in the lowered AST.");
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
    expression->type = expression->expression->type;

    if (m_functionStack.back()) {
        ast::FunctionDefinition *def = m_functionStack.back();
        if (!def->returnType->type->isCompatible(expression->type)) {
            throw errors::TypeMismatchError(expression, def->returnType);
        }
    } else {
        throw errors::TypeMismatchError(expression, nullptr);
    }
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
    SymbolTable::Symbol *symbol = m_namespace->lookup(parameter, parameter->name->value);

    parameter->typeNode->accept(this);

    parameter->type = instance_type(parameter->typeNode);
    symbol->type = parameter->type;
}

void Inferrer::visit(ast::VariableDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->value);

    Types::Type *type = nullptr;

    definition->expression->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
        type = definition->typeNode->type;
    } else {
        type = definition->expression->type;
    }

    if (!type) {
        throw errors::TypeInferenceError(definition);
    }

    symbol->type = type;
    definition->type = type;
}

void Inferrer::visit(ast::FunctionDefinition *definition) {
    SymbolTable::Symbol *functionSymbol = m_namespace->lookup(definition->name);
    Types::Function *function = static_cast<Types::Function *>(functionSymbol->type);

    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup_by_node(definition);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    std::vector<Types::Type *> parameterTypes;
    std::vector<std::string> officialParameterOrder;
    for (auto parameter : definition->parameters) {
        parameter->accept(this);

        parameterTypes.push_back(parameter->type);
        officialParameterOrder.push_back(parameter->name->value);
    }

    definition->returnType->accept(this);
    definition->returnType->type = instance_type(definition->returnType);

    Types::Method *method = new Types::Method(parameterTypes, definition->returnType->type,
                                              officialParameterOrder);

    function->add_method(method);

    functionSymbol->nameSpace->rename(symbol, method->mangled_name());

    symbol->type = method;
    definition->type = method;

    m_functionStack.push_back(definition);

    definition->code->accept(this);

    assert(m_functionStack.back() == definition);
    m_functionStack.pop_back();

    m_namespace = oldNamespace;
}

void Inferrer::visit(ast::TypeDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->value);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    std::vector<Types::Parameter *> input_parameters;
    for (auto t : definition->name->parameters) {
        t->accept(this);

        auto param = dynamic_cast<Types::Parameter *>(t->type);
        if (param == nullptr) {
            throw errors::InvalidTypeParameters(t, 0, 0);
        }

        input_parameters.push_back(param);
    }

    Types::Type *type;
    if (definition->alias) {
        std::vector<Types::Type *> outputParameters;
        for (auto t : definition->alias->parameters) {
            t->accept(this);
            outputParameters.push_back(t->type);
        }

        auto type_constructor = find_type_constructor(definition, definition->alias->value);
        definition->alias->type = type_constructor;
        type = new Types::AliasConstructor(definition, type_constructor,
                                           input_parameters, outputParameters);
    } else {
        std::vector<std::string> field_names;
        std::vector<Types::Constructor *> field_types;
        std::vector<std::vector<Types::Type *> > field_parameters;

        for (auto name : definition->field_names) {
            field_names.push_back(name->value);
        }

        for (auto type : definition->field_types) {
            auto type_constructor = find_type_constructor(type, type->value);

            std::vector<Types::Type *> parameters;
            for (auto t : type->parameters) {
                t->accept(this);
                parameters.push_back(t->type);
            }

            field_types.push_back(type_constructor);
            field_parameters.push_back(parameters);

            type->type = type_constructor;
        }

        type = new Types::RecordConstructor(input_parameters,
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
    statement->type = new Types::Void();
}

void Inferrer::visit(ast::SourceFile *module) {
    module->code->accept(this);
    module->type = module->code->type;
}

Checker::Checker(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Checker::~Checker() {

}

void Checker::check_types(ast::Node *lhs, ast::Node *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    bool compatible = lhs->type->isCompatible(rhs->type);
    if (!compatible) {
        throw errors::TypeMismatchError(lhs, rhs);
    }
}

void Checker::check_not_null(ast::Node *node) {
    if (!node->type) {
        throw errors::InternalError(node, "No type given for: " + Token::rule_string(node->token->rule));
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

void Checker::visit(ast::Argument *argument) {
    if (argument->name) {
        argument->name->accept(this);
    }

    argument->value->accept(this);

    check_not_null(argument);
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

    // SymbolTable::Symbol *symbol = m_namespace->lookup(expression->lhs);
    throw errors::ConstantAssignmentError(expression->lhs);
}

void Checker::visit(ast::Selector *expression) {
    expression->operand->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Index *expression) {
    expression->operand->accept(this);
    expression->index->accept(this);
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
    throw errors::InternalAstError(expression);
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

    if (parameter->defaultExpression) {
        parameter->defaultExpression->accept(this);
    }

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

    SymbolTable::Symbol *functionSymbol = m_namespace->lookup(definition->name);

    Types::Method *method = static_cast<Types::Method *>(definition->type);

    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup(definition, method->mangled_name());

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

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

    SymbolTable::Symbol *symbol = m_namespace->lookup(definition->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
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
