//
// Created by Thomas Leese on 12/01/2017.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "types.h"

#include "ast.h"

using namespace acorn;
using namespace acorn::ast;

Node::Node(Token token) : m_token(token) {

}

Node::~Node() {

}

acorn::Token Node::token() const {
    return m_token;
}

Expression::Expression(Token token) : Node(token), m_type(nullptr) {

}

acorn::types::Type *Expression::type() const {
    return m_type;
}

void Expression::set_type(acorn::types::Type *type) {
    m_type = type;
}

bool Expression::has_type() const {
    return m_type != nullptr;
}

void Expression::copy_type_from(Expression *expression) {
    // TODO set a warning here if the type is null?
    set_type(expression->type());
}

bool Expression::has_compatible_type_with(Expression *expression) const {
    // TODO set a warning here if the type is null?
    return m_type->is_compatible(expression->type());
}

std::string Expression::type_name() const {
    if (m_type) {
        return m_type->name();
    } else {
        return "null";
    }
}

void Block::accept(Visitor *visitor) {
    visitor->visit(this);
}

Name::Name(Token token) : Expression(token) {

}

Name::Name(Token token, std::string name) : Expression(token), m_value(name) {

}

bool Name::has_parameters() const {
    return !m_parameters.empty();
}

std::string Name::collapsed_value() const {
    std::stringstream ss;
    ss << m_value;
    if (has_parameters()) {
        ss << "_";
        for (auto p : m_parameters) {
            ss << p->collapsed_value() << "_";
        }
    }
    return ss.str();
}

void Name::collapse_parameters() {
    m_value = collapsed_value();
    m_parameters.clear();
}

std::string Name::value() const {
    return m_value;
}

std::vector<Name *> Name::parameters() const {
    return m_parameters;
}

void Name::add_parameter(Name *identifier) {
    m_parameters.push_back(identifier);
}

void Name::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::vector<Expression *> Block::expressions() const {
    return m_expressions;
}

void Block::add_expression(Expression *expression) {
    m_expressions.push_back(expression);
}

void Block::insert_expression(int index, Expression *expression) {
    m_expressions.insert(m_expressions.begin() + index, expression);
}

bool Block::empty() const {
    return m_expressions.empty();
}

VariableDeclaration::VariableDeclaration(Token token, Name *name, Name *type, bool builtin) :
        Expression(token), m_name(name), m_given_type(type), m_builtin(builtin) {

}

Name *VariableDeclaration::name() const {
    return m_name.get();
}

bool VariableDeclaration::has_given_type() {
    return m_given_type != nullptr;
}

Name *VariableDeclaration::given_type() const {
    return m_given_type.get();
}

bool VariableDeclaration::builtin() const {
    return m_builtin;
}

void VariableDeclaration::set_type(types::Type *type) {
    Expression::set_type(type);
    m_name->set_type(type);
}

void VariableDeclaration::accept(Visitor *visitor) {
    visitor->visit(this);
}

Int::Int(Token token, std::string value) : Expression(token), m_value(value) {

}

std::string Int::value() const {
    return m_value;
}

void Int::accept(Visitor *visitor) {
    visitor->visit(this);
}

Float::Float(Token token, std::string value) : Expression(token), m_value(value) {

}

std::string Float::value() const {
    return m_value;
}

void Float::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Complex::accept(Visitor *visitor) {
    visitor->visit(this);
}

String::String(Token token, std::string value) : Expression(token), m_value(value) {

}

std::string String::value() const {
    return m_value;
}

void String::accept(Visitor *visitor) {
    visitor->visit(this);
}

List::List(Token token, std::vector<Expression *> elements) : Expression(token) {
    for (auto &element : elements) {
        m_elements.push_back(std::unique_ptr<Expression>(element));
    }
}

std::vector<Expression *> List::elements() const {
    std::vector<Expression *> elements;
    for (auto &element : m_elements) {
        elements.push_back(element.get());
    }
    return elements;
}

Expression *List::get_element(size_t index) const {
    return m_elements[index].get();
}

size_t List::no_elements() const {
    return m_elements.size();
}

void List::accept(Visitor *visitor) {
    visitor->visit(this);
}

Dictionary::Dictionary(Token token, std::vector<Expression *> keys, std::vector<Expression *> values) : Expression(token) {
    assert(keys.size() == values.size());

    for (auto &key : keys) {
        m_keys.push_back(std::unique_ptr<Expression>(key));
    }

    for (auto &value : values) {
        m_values.push_back(std::unique_ptr<Expression>(value));
    }
}

std::vector<Expression *> Dictionary::keys() const {
    std::vector<Expression *> keys;
    for (auto &key : m_keys) {
        keys.push_back(key.get());
    }
    return keys;
}

Expression *Dictionary::get_key(size_t index) const {
    return m_keys[index].get();
}

std::vector<Expression *> Dictionary::values() const {
    std::vector<Expression *> values;
    for (auto &value : m_values) {
        values.push_back(value.get());
    }
    return values;
}

Expression *Dictionary::get_value(size_t index) const {
    return m_values[index].get();
}

size_t Dictionary::no_elements() const {
    assert(m_keys.size() == m_values.size());
    return m_keys.size();
}

void Dictionary::accept(Visitor *visitor) {
    visitor->visit(this);
}

Tuple::Tuple(Token token, std::vector<Expression *> elements) :
        Expression(token)
{
    for (auto e : elements) {
        m_elements.push_back(std::unique_ptr<Expression>(e));
    }
}

std::vector<Expression *> Tuple::elements() {
    std::vector<Expression *> elements;
    for (auto &e : m_elements) {
        elements.push_back(e.get());
    }
    return elements;
}

void Tuple::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call::Call(Token token, Expression *operand) : Expression(token), m_method_index(0), m_method_specialisation_index(0) {
    this->operand = operand;
}

Call::Call(Token token, std::string name, Expression *arg1, Expression *arg2) : Call(token, new Name(token, name)) {
    if (arg1) {
        m_positional_arguments.push_back(std::unique_ptr<Expression>(arg1));
    }

    if (arg2) {
        m_positional_arguments.push_back(std::unique_ptr<Expression>(arg2));
    }
}

std::vector<Expression *> Call::positional_arguments() const {
    std::vector<Expression *> args;
    for (auto &arg : m_positional_arguments) {
        args.push_back(arg.get());
    }
    return args;
}

std::vector<types::Type *> Call::positional_argument_types() const {
    std::vector<types::Type *> types;
    for (auto &arg : m_positional_arguments) {
        types.push_back(arg->type());
    }
    return types;
}

void Call::add_positional_argument(Expression *argument) {
    m_positional_arguments.push_back(std::unique_ptr<Expression>(argument));
}

std::map<std::string, Expression *> Call::keyword_arguments() const {
    std::map<std::string, Expression *> args;
    for (auto const &entry : m_keyword_arguments) {
        args[entry.first] = entry.second.get();
    }
    return args;
}

std::map<std::string, types::Type *> Call::keyword_argument_types() const {
    std::map<std::string, types::Type *> types;
    for (auto const &entry : m_keyword_arguments) {
        types[entry.first] = entry.second->type();
    }
    return types;
}

void Call::add_keyword_argument(std::string name, Expression *argument) {
    m_keyword_arguments[name] = std::unique_ptr<Expression>(argument);
}

void Call::set_method_index(int index) {
    m_method_index = index;
}

int Call::get_method_index() const {
    return m_method_index;
}

void Call::set_method_specialisation_index(int index) {
    m_method_specialisation_index = index;
}

int Call::get_method_specialisation_index() const {
    return m_method_specialisation_index;
}

void Call::accept(Visitor *visitor) {
    visitor->visit(this);
}

CCall::CCall(Token token, Name *name, std::vector<Name *> parameters, Name *given_return_type, std::vector<Expression *> arguments) : Expression(token), m_name(name), m_given_return_type(given_return_type) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::unique_ptr<Name>(parameter));
    }

    for (auto &argument : arguments) {
        m_arguments.push_back(std::unique_ptr<Expression>(argument));
    }
}

Name *CCall::name() const {
    return m_name.get();
}

std::vector<Name *> CCall::parameters() const {
    std::vector<Name *> parameters;
    for (auto &parameter : m_parameters) {
        parameters.push_back(parameter.get());
    }
    return parameters;
}

Name *CCall::given_return_type() const {
    return m_given_return_type.get();
}

std::vector<Expression *> CCall::arguments() const {
    std::vector<Expression *> arguments;
    for (auto &argument : m_arguments) {
        arguments.push_back(argument.get());
    }
    return arguments;
}

void CCall::accept(Visitor *visitor) {
    visitor->visit(this);
}

Cast::Cast(Token token) : Expression(token) {
    this->operand = nullptr;
    this->new_type = nullptr;
}

Cast::Cast(Token token, Expression *operand, Name *new_type) : Expression(token) {
    this->operand = operand;
    this->new_type = new_type;
}

void Cast::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Token token, VariableDeclaration *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

bool Assignment::builtin() const {
    return lhs->builtin();
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

Selector::Selector(Token token, Expression *operand, Name *field) :
        Expression(token),
        operand(operand),
        name(field)
{

}

Selector::Selector(Token token, Expression *operand, std::string field) :
        Selector(token, operand, new Name(token, field))
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

Return::Return(Token token) : Expression(token), expression(nullptr) {

}

Return::Return(Token token, Expression *expression) : Expression(token) {
    this->expression = expression;
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
        Expression(token), m_condition(condition), m_assignment(assignment), m_body(body)
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

Let::Let(Token token) :
        Expression(token),
        assignment(nullptr),
        m_body(nullptr)
{
    // intentionally empty
}

Let::Let(Token token, std::string name, Expression *value, Expression *body) :
        Expression(token),
        m_body(body)
{
    auto variable_declaration = new VariableDeclaration(token, new Name(token, name), nullptr, false);
    this->assignment = new Assignment(token, variable_declaration, value);
}

Expression *Let::body() const {
    return m_body.get();
}

bool Let::has_body() const {
    return m_body != nullptr;
}

void Let::set_body(Expression *body) {
    m_body.reset(body);
}

void Let::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(Token token, bool inout, Name *name, Name *given_type) : Expression(token), m_inout(inout), m_name(name), m_given_type(given_type) {

}

bool Parameter::inout() const {
    return m_inout;
}

Name *Parameter::name() const {
    return m_name.get();
}

Name *Parameter::given_type() const {
    return m_given_type.get();
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

Def::Def(Token token, Expression *name, bool builtin, std::vector<Parameter *> parameters, Expression *body, Name *given_return_type) : Expression(token), m_name(name), m_builtin(builtin), m_body(body), m_given_return_type(given_return_type) {
    for (auto parameter : parameters) {
        m_parameters.push_back(std::unique_ptr<Parameter>(parameter));
    }
}

Expression *Def::name() const {
    return m_name.get();
}

bool Def::builtin() const {
    return m_builtin;
}

std::vector<Parameter *> Def::parameters() const {
    std::vector<Parameter *> parameters;
    for (auto &parameter : m_parameters) {
        parameters.push_back(parameter.get());
    }
    return parameters;
}

Parameter *Def::get_parameter(size_t index) const {
    return m_parameters[index].get();
}

size_t Def::no_parameters() const {
    return m_parameters.size();
}

Expression *Def::body() const {
    return m_body.get();
}

Name *Def::given_return_type() const {
    return m_given_return_type.get();
}

bool Def::has_given_return_type() const {
    return m_given_return_type != nullptr;
}

void Def::set_type(types::Type *type) {
    Expression::set_type(type);
    m_name->set_type(type);
}

void Def::accept(Visitor *visitor) {
    visitor->visit(this);
}

Type::Type(Token token, Name *name, bool builtin) : Expression(token), m_name(name), m_builtin(builtin), alias(nullptr) {

}

Name *Type::name() const {
    return m_name.get();
}

void Type::set_name(Name *name) {
    m_name.reset(name);
}

void Type::set_type(types::Type *type) {
    Expression::set_type(type);
    m_name->set_type(type);
}

bool Type::builtin() const {
    return m_builtin;
}

void Type::set_builtin(bool builtin) {
    m_builtin = builtin;
}

void Type::accept(Visitor *visitor) {
    visitor->visit(this);
}

MethodSignature::MethodSignature(Token token, Name *name, std::vector<Name *> parameter_types, Name *return_type) :
        Node(token),
        m_name(name),
        m_return_type(return_type)
{
    for (auto p : parameter_types) {
        m_parameter_types.push_back(std::unique_ptr<Name>(p));
    }
}

Name *MethodSignature::name() const {
    return m_name.get();
}

std::vector<Name *> MethodSignature::parameter_types() const {
    std::vector<Name *> result;
    for (auto &type : m_parameter_types) {
        result.push_back(type.get());
    }
    return result;
}

Name *MethodSignature::return_type() const {
    return m_return_type.get();
}

void MethodSignature::accept(Visitor *visitor) {

}

Module::Module(Token token, Name *name, Block *body) : Expression(token), m_name(name), m_body(body) {

}

Name *Module::name() const {
    return m_name.get();
}

Block *Module::body() const {
    return m_body.get();
}

void Module::accept(Visitor *visitor) {
    visitor->visit(this);
}

Import::Import(Token token, String *path) : Expression(token) {
    this->path = path;
}

void Import::accept(Visitor *visitor) {
    visitor->visit(this);
}

SourceFile::SourceFile(Token token, std::string name) : Expression(token), name(name) {
    this->code = new Block(token);
}

void SourceFile::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor()
{

}
