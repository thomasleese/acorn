//
// Created by Thomas Leese on 12/01/2017.
//

#include <sstream>

#include "ast.h"

using namespace acorn::ast;

Node::Node(Token token) : type(nullptr), m_token(token) {

}

Node::~Node() {

}

acorn::Token Node::token() const {
    return m_token;
}

void Block::accept(Visitor *visitor) {
    visitor->visit(this);
}

Identifier::Identifier(Token token) : Expression(token) {

}

Identifier::Identifier(Token token, std::string name) : Expression(token) {
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

VariableDeclaration::VariableDeclaration(Token token, Identifier *name, Identifier *type) :
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

TupleLiteral::TupleLiteral(Token token, std::vector<Expression *> elements) :
        Expression(token)
{
    for (auto e : elements) {
        m_elements.push_back(std::unique_ptr<Expression>(e));
    }
}

std::vector<Expression *> TupleLiteral::elements() {
    std::vector<Expression *> elements;
    for (auto &e : m_elements) {
        elements.push_back(e.get());
    }
    return elements;
}

void TupleLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call::Call(Token token) : Expression(token), operand(nullptr) {

}

Call::Call(Token token, std::string name, Expression *arg1, Expression *arg2) : Call(token) {
    this->operand = new Identifier(token, name);

    if (arg1) {
        this->arguments.push_back(arg1);
    }

    if (arg2) {
        this->arguments.push_back(arg2);
    }
}

void Call::accept(Visitor *visitor) {
    visitor->visit(this);
}

CCall::CCall(Token token) : Expression(token) {
    this->name = nullptr;
    this->returnType = nullptr;
}

void CCall::accept(Visitor *visitor) {
    visitor->visit(this);
}

Cast::Cast(Token token) : Expression(token) {
    this->operand = nullptr;
    this->new_type = nullptr;
}

void Cast::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Token token, Expression *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

Selector::Selector(Token token, Expression *operand, Identifier *field) :
        Expression(token),
        operand(operand),
        name(field)
{

}

Selector::Selector(Token token, Expression *operand, std::string field) :
        Selector(token, operand, new Identifier(token, field))
{

}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

While::While(Token token, Expression *condition, Expression *body) :
        Expression(token),
        m_condition(condition),
        m_body(body)
{
    // intentionally left empty
}

Expression *While::condition() const {
    return m_condition.get();
}

Expression *While::body() const {
    return m_body.get();
}

void While::accept(Visitor *visitor) {
    visitor->visit(this);
}

void If::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Return::accept(Visitor *visitor) {
    visitor->visit(this);
}

Spawn::Spawn(Token token, Call *call) : Expression(token) {
    this->call = call;
}

void Spawn::accept(Visitor *visitor) {
    visitor->visit(this);
}

Case::Case(Token token, Expression *condition, Expression *assignment, Expression *body) :
        Node(token), m_condition(condition), m_assignment(assignment), m_body(body)
{

}

Expression *Case::condition() const {
    return m_condition.get();
}

Expression *Case::assignment() const {
    return m_assignment.get();
}

Expression *Case::body() const {
    return m_body.get();
}

void Case::accept(Visitor *visitor) {
    // visitor->visit(this);
}

Switch::Switch(Token token, Expression *expression, std::vector<Case *> cases, Expression *default_case) :
        Expression(token), m_expression(expression), m_default_case(default_case)
{
    for (auto entry : cases) {
        m_cases.push_back(std::unique_ptr<Case>(entry));
    }
}

Expression *Switch::expression() const {
    return m_expression.get();
}

std::vector<Case *> Switch::cases() const {
    std::vector<Case *> cases;
    for (auto &entry : m_cases) {
        cases.push_back(entry.get());
    }
    return cases;
}

Expression *Switch::default_case() const {
    return m_default_case.get();
}

void Switch::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(Token token) : Node(token), name(nullptr), typeNode(nullptr) {

}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDefinition::VariableDefinition(Token token) :
        Definition(token),
        assignment(nullptr)
{
    // intentionally empty
}

VariableDefinition::VariableDefinition(Token token, std::string name, Expression *value) :
        VariableDefinition(token)
{
    this->name = new Identifier(token, name);
    this->assignment = new Assignment(token, new VariableDeclaration(token, this->name), value);
}

void VariableDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

FunctionDefinition::FunctionDefinition(Token token) :
        Definition(token),
        returnType(nullptr)
{
    // intentionally empty
}

void FunctionDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

TypeDefinition::TypeDefinition(Token token) : Definition(token) {
    this->alias = nullptr;
    this->name = nullptr;
}

void TypeDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

MethodSignature::MethodSignature(Token token, Identifier *name, std::vector<Identifier *> parameter_types, Identifier *return_type) :
        Node(token),
        m_name(name),
        m_return_type(return_type)
{
    for (auto p : parameter_types) {
        m_parameter_types.push_back(std::unique_ptr<Identifier>(p));
    }
}

Identifier *MethodSignature::name() const {
    return m_name.get();
}

std::vector<Identifier *> MethodSignature::parameter_types() const {
    std::vector<Identifier *> result;
    for (auto &type : m_parameter_types) {
        result.push_back(type.get());
    }
    return result;
}

Identifier *MethodSignature::return_type() const {
    return m_return_type.get();
}

void MethodSignature::accept(Visitor *visitor) {

}

DefinitionExpression::DefinitionExpression(Definition *definition) : Expression(definition->token()) {
    this->definition = definition;
}

void DefinitionExpression::accept(Visitor *visitor) {
    visitor->visit(this);
}

ImportExpression::ImportExpression(Token token, StringLiteral *path) : Expression(token) {
    this->path = path;
}

void ImportExpression::accept(Visitor *visitor) {
    visitor->visit(this);
}

SourceFile::SourceFile(Token token, std::string name) : Node(token) {
    this->name = name;
    this->code = new Block(token);
}

void SourceFile::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor()
{

}
