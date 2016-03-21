//
// Created by Thomas Leese on 21/03/2016.
//

#include <iostream>

#include "Typing.h"
#include "Errors.h"

#include "SymbolTable.h"

using namespace Typing;

Inferrer::Inferrer(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Inferrer::~Inferrer() {

}

Types::Type *Inferrer::find_type(AST::Node *node, std::string name) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(node, name);
    Types::TypeType *typeType = static_cast<Types::TypeType *>(symbol->type);
    return typeType->type;
}

void Inferrer::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Inferrer::visit(AST::Identifier *expression) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(expression, expression->name);
    expression->type = symbol->type;
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

}

void Inferrer::visit(AST::MappingLiteral *mapping) {

}

void Inferrer::visit(AST::Argument *argument) {

}

void Inferrer::visit(AST::Call *expression) {

}

void Inferrer::visit(AST::Assignment *expression) {

}

void Inferrer::visit(AST::Selector *expression) {

}

void Inferrer::visit(AST::While *expression) {

}

void Inferrer::visit(AST::For *expression) {

}

void Inferrer::visit(AST::If *expression) {

}

void Inferrer::visit(AST::Type *type) {
    type->type = find_type(type, type->name->name);
}

void Inferrer::visit(AST::Cast *cast) {
    cast->typeNode->accept(this);
    cast->type = cast->typeNode->type;
}

void Inferrer::visit(AST::Parameter *parameter) {

}

void Inferrer::visit(AST::VariableDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name);

    definition->cast->accept(this);
    definition->expression->accept(this);

    Types::Type *type = definition->cast->type;
    symbol->type = type;
    definition->type = type;
}

void Inferrer::visit(AST::FunctionDefinition *definition) {

}

void Inferrer::visit(AST::TypeDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name->name);

    Types::Type *actualType;
    if (definition->alias) {
        actualType = find_type(definition, definition->alias->name->name);
    } else {
        Types::Record *record = new Types::Record();
        actualType = record;
    }

    Types::TypeType *type = new Types::TypeType(actualType);
    symbol->type = type;
    definition->type = type;
}

void Inferrer::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Inferrer::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Inferrer::visit(AST::Module *module) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(module, module->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    module->code->accept(this);

    m_namespace = oldNamespace;
}

Checker::Checker(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Checker::~Checker() {

}

void Checker::check_types(AST::Node *lhs, AST::Node *rhs) {
    if (lhs->type != rhs->type) {
        throw Errors::TypeMismatchError(lhs, rhs);
    }
}

void Checker::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Checker::visit(AST::Identifier *expression) {

}

void Checker::visit(AST::BooleanLiteral *boolean) {

}

void Checker::visit(AST::IntegerLiteral *expression) {

}

void Checker::visit(AST::FloatLiteral *expression) {

}

void Checker::visit(AST::ImaginaryLiteral *imaginary) {

}

void Checker::visit(AST::StringLiteral *expression) {

}

void Checker::visit(AST::SequenceLiteral *sequence) {

}

void Checker::visit(AST::MappingLiteral *mapping) {

}

void Checker::visit(AST::Argument *argument) {

}

void Checker::visit(AST::Call *expression) {

}

void Checker::visit(AST::Assignment *expression) {

}

void Checker::visit(AST::Selector *expression) {

}

void Checker::visit(AST::While *expression) {

}

void Checker::visit(AST::For *expression) {

}

void Checker::visit(AST::If *expression) {

}

void Checker::visit(AST::Type *type) {

}

void Checker::visit(AST::Cast *cast) {

}

void Checker::visit(AST::Parameter *parameter) {

}

void Checker::visit(AST::VariableDefinition *definition) {
    check_types(definition, definition->expression);
}

void Checker::visit(AST::FunctionDefinition *definition) {

}

void Checker::visit(AST::TypeDefinition *definition) {

}

void Checker::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Checker::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Checker::visit(AST::Module *module) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(module, module->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    module->code->accept(this);

    m_namespace = oldNamespace;
}
