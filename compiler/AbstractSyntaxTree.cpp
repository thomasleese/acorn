//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "Lexer.h"
#include "AbstractSyntaxTree.h"

using namespace AST;

Node::Node(Lexer::Token *token) {
    this->token = token;
}

Node::~Node() {

}

void CodeBlock::accept(Visitor *visitor) {
    visitor->visit(this);
}

Identifier::Identifier(Lexer::Token *token) : Expression(token) {

}

Identifier::Identifier(Lexer::Token *token, std::string name) : Expression(token) {
    this->name = name;
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

Argument::Argument(Lexer::Token *token) : Node(token) {

}

Argument::Argument(Lexer::Token *token, std::string name) : Node(token) {
    this->name = new Identifier(token, name);
}

void Argument::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Call::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Lexer::Token *token, Expression *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

void While::accept(Visitor *visitor) {
    visitor->visit(this);
}

void For::accept(Visitor *visitor) {
    visitor->visit(this);
}

void If::accept(Visitor *visitor) {
    visitor->visit(this);
}

void TypeDeclaration::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Parameter::accept(Visitor *visitor) {
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

DefinitionStatement::DefinitionStatement(Definition *definition) : Statement(definition->token) {
    this->definition = definition;
}

void DefinitionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

ExpressionStatement::ExpressionStatement(Expression *expression) : Statement(expression->token) {
    this->expression = expression;
}

void ExpressionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

Module::Module(Lexer::Token *token, std::string name) : Node(token) {
    this->name = name;
    this->code = new CodeBlock(token);
}

void Module::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}
