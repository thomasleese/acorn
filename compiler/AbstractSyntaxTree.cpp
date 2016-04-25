//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"

#include "AbstractSyntaxTree.h"

using namespace AST;

Node::Node(Token *token) {
    this->token = token;
    this->type = nullptr;
}

Node::~Node() {

}

void CodeBlock::accept(Visitor *visitor) {
    visitor->visit(this);
}

Identifier::Identifier(Token *token) : Expression(token) {

}

Identifier::Identifier(Token *token, std::string name) : Expression(token) {
    this->name = name;
}

void Identifier::accept(Visitor *visitor) {
    visitor->visit(this);
}

void BooleanLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void IntegerLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void FloatLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void ImaginaryLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void StringLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void SequenceLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void MappingLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

Argument::Argument(Token *token) : Node(token) {
    this->name = nullptr;
}

Argument::Argument(Token *token, std::string name) : Node(token) {
    this->name = new Identifier(token, name);
}

void Argument::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call::Call(Token *token) : Expression(token) {

}

Call::Call(std::string name, Token *token) : Expression(token) {
    this->operand = new Identifier(token, name);
}

void Call::accept(Visitor *visitor) {
    visitor->visit(this);
}

CCall::CCall(Token *token) : Expression(token) {
    this->name = nullptr;
    this->returnType = nullptr;
}

void CCall::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Token *token, Identifier *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Index::accept(Visitor *visitor) {
    visitor->visit(this);
}

Comma::Comma(Token *token) : Expression(token) {
    this->lhs = nullptr;
    this->rhs = nullptr;
}

Comma::Comma(Expression *lhs, Expression *rhs, Token *token) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Comma::accept(Visitor *visitor) {
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

void Return::accept(Visitor *visitor) {
    visitor->visit(this);
}

Spawn::Spawn(Token *token, Call *call) : Expression(token) {
    this->call = call;
}

void Spawn::accept(Visitor *visitor) {
    visitor->visit(this);
}

Type::Type(Token *token) : Expression(token) {

}

Type::Type(std::string name, Token *token) : Expression(token) {
    this->name = new Identifier(token, name);
}

void Type::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(Token *token) : Node(token) {
    this->defaultExpression = 0;
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDefinition::VariableDefinition(Token *token) : Definition(token) {

}

VariableDefinition::VariableDefinition(std::string name, Token *token) : Definition(token) {
    this->name = new Identifier(token, name);
}

void VariableDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

void FunctionDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

TypeDefinition::TypeDefinition(Token *token) : Definition(token) {
    this->alias = nullptr;
    this->name = nullptr;
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

ImportStatement::ImportStatement(Token *token, StringLiteral *path) : Statement(token) {
    this->path = path;
}

void ImportStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

SourceFile::SourceFile(Token *token, std::string name) : Node(token) {
    this->name = name;
    this->code = new CodeBlock(token);
}

void SourceFile::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}
