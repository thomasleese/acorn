//
// Created by Thomas Leese on 07/05/2016.
//

#include <sstream>

#include "visitor.h"

#include "nodes.h"

using namespace acorn::ast;

Node::Node(Token *token) : token(token), type(nullptr) {

}

Node::~Node() {

}

void CodeBlock::accept(Visitor *visitor) {
    visitor->visit(this);
}

Identifier::Identifier(Token *token) : Expression(token) {

}

Identifier::Identifier(Token *token, std::string name) : Expression(token) {
    this->value = name;
}

bool Identifier::has_parameters() const {
    return !parameters.empty();
}

std::string Identifier::collapsed_value() const {
    std::stringstream ss;
    ss << this->value;
    if (has_parameters()) {
        ss << "_";
        for (auto p : this->parameters) {
            ss << p->collapsed_value() << "_";
        }
    }
    return ss.str();
}

void Identifier::collapse_parameters() {
    this->value = collapsed_value();
    this->parameters.clear();
}

void Identifier::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDeclaration::VariableDeclaration(Token *token, Identifier *name, Identifier *type) :
        Expression(token), m_name(name), m_given_type(type) {

}

Identifier *VariableDeclaration::name() const {
    return m_name.get();
}

bool VariableDeclaration::has_given_type() {
    return m_given_type != nullptr;
}

Identifier *VariableDeclaration::given_type() const {
    return m_given_type.get();
}

void VariableDeclaration::accept(Visitor *visitor) {
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

void RecordLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call::Call(Token *token) : Expression(token), operand(nullptr) {

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

Cast::Cast(Token *token) : Expression(token) {
    this->operand = nullptr;
    this->new_type = nullptr;
}

void Cast::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Token *token, Expression *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Selector::accept(Visitor *visitor) {
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

Sizeof::Sizeof(Token *token, Identifier *identifier) : Expression(token) {
    this->identifier = identifier;
}

void Sizeof::accept(Visitor *visitor) {
    visitor->visit(this);
}

Strideof::Strideof(Token *token, Identifier *identifier) : Expression(token) {
    this->identifier = identifier;
}

void Strideof::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(Token *token) : Node(token), name(nullptr), typeNode(nullptr) {

}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDefinition::VariableDefinition(Token *token) :
        Definition(token),
        assignment(nullptr)
{
    // intentionally empty
}

VariableDefinition::VariableDefinition(std::string name, Token *token) : VariableDefinition(token) {
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
