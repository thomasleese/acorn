//
// Created by Thomas Leese on 21/03/2016.
//

#include <iostream>
#include <set>
#include <sstream>

#include "Typing.h"
#include "Errors.h"

#include "SymbolTable.h"

using namespace Typing;

Inferrer::Inferrer(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Inferrer::~Inferrer() {

}

Types::Constructor *Inferrer::find_type_constructor(AST::Node *node, std::string name) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(node, name);
    Types::Constructor *typeConstructor = dynamic_cast<Types::Constructor *>(symbol->type);
    if (typeConstructor) {
        return typeConstructor;
    } else {
        return nullptr;
    }
}

Types::Parameter *Inferrer::find_type_parameter(AST::Node *node, std::string name) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(node, name);
    Types::Parameter *typeParameter = dynamic_cast<Types::Parameter *>(symbol->type);
    if (typeParameter) {
        return typeParameter;
    } else {
        return nullptr;
    }
}

Types::Type *Inferrer::find_type(AST::Node *node, std::string name, std::vector<AST::Type *> parameters) {
    std::vector<Types::Type *> parameterTypes;

    for (auto parameter : parameters) {
        parameter->accept(this);
        parameterTypes.push_back(parameter->type);
    }

    Types::Constructor *typeConstructor = find_type_constructor(node, name);
    if (typeConstructor != nullptr) {
        return typeConstructor->create(node, parameterTypes);
    } else {
        Types::Parameter *typeParameter = find_type_parameter(node, name);
        if (typeParameter != nullptr) {
            return typeParameter;
        } else {
            throw Errors::InvalidTypeConstructor(node);
        }
    }
}

Types::Type *Inferrer::find_type(AST::Node *node, std::string name) {
    return find_type(node, name, std::vector<AST::Type *>());
}

Types::Type *Inferrer::find_type(AST::Type *type) {
    return find_type(type, type->name->name, type->parameters);
}

void Inferrer::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }

    if (block->statements.empty()) {
        block->type = new Types::Void();
    } else {
        block->type = block->statements.back()->type;
    }
}

void Inferrer::visit(AST::Identifier *expression) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(expression, expression->name);
    expression->type = symbol->type;

    if (!symbol->type) {
        throw Errors::UndefinedError(expression, expression->name);
    }
}

void Inferrer::visit(AST::Type *type) {
    type->type = find_type(type);
}

void Inferrer::visit(AST::BooleanLiteral *expression) {
    expression->type = find_type(expression, "Boolean");
}

void Inferrer::visit(AST::IntegerLiteral *expression) {
    expression->type = find_type(expression, "Integer");
}

void Inferrer::visit(AST::FloatLiteral *expression) {
    expression->type = find_type(expression, "Float");
}

void Inferrer::visit(AST::ImaginaryLiteral *expression) {
    expression->type = find_type(expression, "Complex");
}

void Inferrer::visit(AST::StringLiteral *expression) {
    expression->type = find_type(expression, "String");
}

void Inferrer::visit(AST::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    std::set<Types::Type *> types;
    for (auto element : sequence->elements) {
        bool inList = false;
        for (auto type : types) {
            if (*type == *(element->type)) {
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

    sequence->type = new Types::Sequence(elementType);
}

void Inferrer::visit(AST::MappingLiteral *mapping) {
    throw Errors::TypeInferenceError(mapping);
}

void Inferrer::visit(AST::Argument *argument) {
    argument->value->accept(this);
    if (argument->name) {
        argument->name->type = argument->value->type;
    }
    argument->type = argument->value->type;
}

void Inferrer::visit(AST::Call *expression) {
    expression->operand->accept(this);

    for (auto arg : expression->arguments) {
        arg->accept(this);
    }

    Types::Function *function = dynamic_cast<Types::Function *>(expression->operand->type);
    if (function == nullptr) {
        expression->type = new Types::Function();
        throw Errors::TypeMismatchError(expression, expression->operand);
    }

    Types::Method *method = function->find_method(expression, expression->arguments);
    expression->type = method->return_type();
}

void Inferrer::visit(AST::CCall *ccall) {
    for (auto param : ccall->parameters) {
        param->accept(this);
    }

    for (auto arg : ccall->arguments) {
        arg->accept(this);
    }

    // TODO check arg and param types match

    ccall->returnType->accept(this);

    ccall->type = ccall->returnType->type;
}

void Inferrer::visit(AST::Assignment *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
    expression->type = expression->rhs->type;
}

void Inferrer::visit(AST::Selector *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::Comma *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::While *expression) {
    expression->condition->accept(this);
    expression->code->accept(this);

    expression->type = expression->code->type;
}

void Inferrer::visit(AST::For *expression) {
    throw Errors::InternalError(expression, "For should never be in the lowered AST.");
}

void Inferrer::visit(AST::If *expression) {
    expression->condition->accept(this);
    expression->trueCode->accept(this);
    if (expression->falseCode) {
        expression->falseCode->accept(this);
    }

    expression->type = expression->trueCode->type;
}

void Inferrer::visit(AST::Return *expression) {
    expression->expression->accept(this);
    expression->type = expression->expression->type;

    if (m_functionStack.back()) {
        AST::FunctionDefinition *def = m_functionStack.back();
        if (!def->returnType->type->isCompatible(expression->type)) {
            throw Errors::TypeMismatchError(expression, def->returnType);
        }
    } else {
        throw Errors::TypeMismatchError(expression, nullptr);
    }
}

void Inferrer::visit(AST::Spawn *expression) {
    expression->call->accept(this);
    expression->type = expression->call->type;
}

void Inferrer::visit(AST::Parameter *parameter) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(parameter, parameter->name->name);

    parameter->typeNode->accept(this);

    parameter->type = parameter->typeNode->type;
    symbol->type = parameter->type;
}

void Inferrer::visit(AST::VariableDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name);

    Types::Type *type = nullptr;

    definition->expression->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
        type = definition->typeNode->type;
    } else {
        type = definition->expression->type;
    }

    if (!type) {
        throw Errors::TypeInferenceError(definition);
    }

    symbol->type = type;
    definition->type = type;
}

void Inferrer::visit(AST::FunctionDefinition *definition) {
    SymbolTable::Symbol *functionSymbol = m_namespace->lookup(definition->name);
    Types::Function *function = static_cast<Types::Function *>(functionSymbol->type);

    // now we have to find the method symbol inside the function namespace
    std::stringstream ss;
    ss << function->no_methods();

    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup(definition, ss.str());

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    std::vector<Types::Type *> parameterTypes;
    std::vector<std::string> officialParameterOrder;
    for (auto parameter : definition->parameters) {
        parameter->accept(this);

        parameterTypes.push_back(parameter->type);
        officialParameterOrder.push_back(parameter->name->name);
    }

    definition->returnType->accept(this);

    Types::Method *method = new Types::Method(parameterTypes, definition->returnType->type,
                                              officialParameterOrder);

    function->add_method(method);

    symbol->type = method;
    definition->type = method;

    m_functionStack.push_back(definition);

    definition->code->accept(this);

    assert(m_functionStack.back() == definition);
    m_functionStack.pop_back();

    m_namespace = oldNamespace;
}

void Inferrer::visit(AST::TypeDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    Types::Type *type;
    if (definition->alias) {
        std::vector<Types::Type *> inputParameters;
        for (auto t : definition->name->parameters) {
            t->accept(this);
            inputParameters.push_back(t->type);
        }

        std::vector<Types::Type *> outputParameters;
        for (auto t : definition->alias->parameters) {
            t->accept(this);
            outputParameters.push_back(t->type);
        }

        auto type_constructor = find_type_constructor(definition, definition->alias->name->name);
        definition->alias->name->type = type_constructor;
        definition->alias->type = type_constructor;
        type = new Types::AliasConstructor(definition, type_constructor,
                                           inputParameters, outputParameters);
    } else {
        type = new Types::RecordConstructor();
    }

    symbol->type = type;
    definition->type = type;

    m_namespace = oldNamespace;
}

void Inferrer::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
    statement->type = statement->definition->type;
}

void Inferrer::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
    statement->type = statement->expression->type;
}

void Inferrer::visit(AST::ImportStatement *statement) {
    statement->path->accept(this);
    statement->type = new Types::Void();
}

void Inferrer::visit(AST::SourceFile *module) {
    module->code->accept(this);
    module->type = module->code->type;
}

Checker::Checker(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Checker::~Checker() {

}

void Checker::check_types(AST::Node *lhs, AST::Node *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    bool compatible = lhs->type->isCompatible(rhs->type);
    if (!compatible) {
        throw Errors::TypeMismatchError(lhs, rhs);
    }
}

void Checker::check_not_null(AST::Node *node) {
    if (!node->type) {
        throw Errors::InternalError(node, "No type given for: " + Token::rule_string(node->token->rule));
    }
}

void Checker::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }

    check_not_null(block);
}

void Checker::visit(AST::Identifier *expression) {
    check_not_null(expression);
}

void Checker::visit(AST::Type *type) {
    for (auto p : type->parameters) {
        p->accept(this);
    }
    check_not_null(type);
}

void Checker::visit(AST::BooleanLiteral *boolean) {
    check_not_null(boolean);
}

void Checker::visit(AST::IntegerLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(AST::FloatLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(AST::ImaginaryLiteral *imaginary) {
    check_not_null(imaginary);
}

void Checker::visit(AST::StringLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(AST::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    check_not_null(sequence);
}

void Checker::visit(AST::MappingLiteral *mapping) {
    for (int i = 0; i < mapping->keys.size(); i++) {
        mapping->keys[i]->accept(this);
        mapping->values[i]->accept(this);
    }

    check_not_null(mapping);
}

void Checker::visit(AST::Argument *argument) {
    if (argument->name) {
        argument->name->accept(this);
    }

    argument->value->accept(this);

    check_not_null(argument);
}

void Checker::visit(AST::Call *expression) {
    expression->operand->accept(this);

    for (auto arg : expression->arguments) {
        arg->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(AST::CCall *ccall) {
    // ccall->name->accept(this);

    for (auto param : ccall->parameters) {
        param->accept(this);
    }

    ccall->returnType->accept(this);

    for (auto arg : ccall->arguments) {
        arg->accept(this);
    }
}

void Checker::visit(AST::Assignment *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
    check_not_null(expression);

    SymbolTable::Symbol *symbol = m_namespace->lookup(expression->lhs);
    if (!symbol->is_mutable) {
        throw Errors::ConstantAssignmentError(expression->lhs);
    }
}

void Checker::visit(AST::Selector *expression) {
    expression->operand->accept(this);
    check_not_null(expression);
}

void Checker::visit(AST::Comma *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
    check_not_null(expression);
}

void Checker::visit(AST::While *expression) {
    expression->condition->accept(this);
    expression->code->accept(this);
    check_not_null(expression);
}

void Checker::visit(AST::For *expression) {
    throw Errors::InternalAstError(expression);
}

void Checker::visit(AST::If *expression) {
    expression->condition->accept(this);
    expression->trueCode->accept(this);
    if (expression->falseCode) {
        expression->falseCode->accept(this);
    }
    check_not_null(expression);
}

void Checker::visit(AST::Return *expression) {
    expression->expression->accept(this);
    check_not_null(expression);
}

void Checker::visit(AST::Spawn *expression) {
    expression->call->accept(this);
    check_not_null(expression);
}

void Checker::visit(AST::Parameter *parameter) {
    parameter->typeNode->accept(this);
    if (parameter->defaultExpression) {
        parameter->defaultExpression->accept(this);
    }
    check_not_null(parameter);
}

void Checker::visit(AST::VariableDefinition *definition) {
    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
    }

    definition->expression->accept(this);

    check_types(definition, definition->expression);
}

void Checker::visit(AST::FunctionDefinition *definition) {
    definition->code->accept(this);

    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

    definition->returnType->accept(this);

    for (auto p : definition->parameters) {
        p->accept(this);
    }

    check_not_null(definition);

    m_namespace = oldNamespace;
}

void Checker::visit(AST::TypeDefinition *definition) {
    // it's valid for the name not to have a type, since it's doesn't exist
    //definition->name->accept(this);

    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        for (auto p : definition->fields) {
            p->accept(this);
        }
    }
    check_not_null(definition);

    m_namespace = oldNamespace;
}

void Checker::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
    check_not_null(statement);
}

void Checker::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
    check_not_null(statement);
}

void Checker::visit(AST::ImportStatement *statement) {
    statement->path->accept(this);
    check_not_null(statement);
}

void Checker::visit(AST::SourceFile *module) {
    module->code->accept(this);
    check_not_null(module);
}
