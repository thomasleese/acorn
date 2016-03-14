//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "Lexer.h"
#include "AbstractSyntaxTree.h"

using namespace AST;

Node::~Node() {

}

void Identifier::accept(Visitor *visitor) {
    visitor->visit(this);
}

void IntegerLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void StringLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void FunctionCall::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

void TypeDeclaration::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

void CodeBlock::accept(Visitor *visitor) {
    visitor->visit(this);
}

void VariableDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

void FunctionDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

void TypeDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

DefinitionStatement::DefinitionStatement(Definition *definition) {
    this->definition = definition;
}

void DefinitionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

ExpressionStatement::ExpressionStatement(Expression *expression) {
    this->expression = expression;
}

void ExpressionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

Module::Module(std::string name) {
    this->name = name;
    this->code = new CodeBlock();
}

void Module::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}
