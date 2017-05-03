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

Node::Node(NodeKind kind, Token token) : m_kind(kind), m_token(token) {

}

Node::Node(Token token) : m_kind(NK_Other), m_token(token) {

}

Node::~Node() {

}

Expression::Expression(NodeKind kind, Token token) : Node(kind, token), m_type(nullptr) {

}

Expression::Expression(Token token) : Node(token), m_type(nullptr) {

}

void Expression::copy_type_from(Expression *expression) {
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

Block::Block(Token token) : Expression(token) {

}

Block::Block(Token token, std::vector<std::unique_ptr<Expression>> expressions) : Expression(token) {
    for (auto &expression : expressions) {
        m_expressions.push_back(std::move(expression));
    }
}

std::vector<Expression *> Block::expressions() const {
    std::vector<Expression *> expressions;
    for (auto &expression : m_expressions) {
        expressions.push_back(expression.get());
    }
    return expressions;
}

void Block::add_expression(std::unique_ptr<Expression> expression) {
    m_expressions.push_back(std::move(expression));
}

void Block::insert_expression(int index, std::unique_ptr<Expression> expression) {
    m_expressions.insert(m_expressions.begin() + index, std::move(expression));
}

void Block::accept(Visitor *visitor) {
    visitor->visit(this);
}

Name::Name(Token token, std::string value) : Expression(NK_Name, token), m_value(value) {

}

Name::Name(Token token, std::string value, std::vector<std::unique_ptr<Name>> parameters) : Name(token, value) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }
}

bool Name::has_parameters() const {
    return !m_parameters.empty();
}

std::string Name::collapsed_value() const {
    std::stringstream ss;
    ss << m_value;
    if (has_parameters()) {
        ss << "_";
        for (auto &parameter : m_parameters) {
            ss << parameter->collapsed_value() << "_";
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
    std::vector<Name *> parameters;
    for (auto &parameter : m_parameters) {
        parameters.push_back(parameter.get());
    }
    return parameters;
}

void Name::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDeclaration::VariableDeclaration(Token token, std::unique_ptr<Name> name, std::unique_ptr<Name> type, bool builtin) : Expression(token), m_name(std::move(name)), m_given_type(std::move(type)), m_builtin(builtin) {

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

List::List(Token token, std::vector<std::unique_ptr<Expression>> elements) : Expression(token) {
    for (auto &element : elements) {
        m_elements.push_back(std::move(element));
    }
}

size_t List::no_elements() const {
    return m_elements.size();
}

Expression &List::element(size_t index) const {
    return *m_elements[index];
}

void List::accept(Visitor *visitor) {
    visitor->visit(this);
}

Dictionary::Dictionary(Token token, std::vector<std::unique_ptr<Expression>> keys, std::vector<std::unique_ptr<Expression>> values) : Expression(token) {
    assert(keys.size() == values.size());

    for (auto &key : keys) {
        m_keys.push_back(std::move(key));
    }

    for (auto &value : values) {
        m_values.push_back(std::move(value));
    }
}

size_t Dictionary::no_elements() const {
    assert(m_keys.size() == m_values.size());
    return m_keys.size();
}

Expression &Dictionary::key(size_t index) const {
    return *m_keys[index];
}

Expression &Dictionary::value(size_t index) const {
    return *m_values[index];
}

void Dictionary::accept(Visitor *visitor) {
    visitor->visit(this);
}

Tuple::Tuple(Token token, std::vector<std::unique_ptr<Expression>> elements) : Expression(token) {
    for (auto &element : elements) {
        m_elements.push_back(std::move(element));
    }
}

void Tuple::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call::Call(Token token, std::unique_ptr<Expression> operand, std::vector<std::unique_ptr<Expression>> positional_arguments, std::map<std::string, std::unique_ptr<Expression>> keyword_arguments) : Expression(NK_Call, token), m_operand(std::move(operand)), m_method_index(0), m_method_specialisation_index(0) {
    for (auto &argument : positional_arguments) {
        m_positional_arguments.push_back(std::move(argument));
    }

    for (auto &entry : keyword_arguments) {
        m_keyword_arguments[entry.first] = std::move(entry.second);
    }
}

Call::Call(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2) : Expression(NK_Call, token), m_operand(std::move(operand)), m_method_index(0), m_method_specialisation_index(0) {
    if (arg1) {
        m_positional_arguments.push_back(std::move(arg1));
    }

    if (arg2) {
        m_positional_arguments.push_back(std::move(arg2));
    }
}

Call::Call(Token token, std::string name, std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2) : Call(token, std::make_unique<Name>(token, name), std::move(arg1), std::move(arg2)) {

}

Call::Call(Token token, std::string name, std::vector<std::unique_ptr<Expression>> arguments) : Call(token, name) {
    for (auto &argument : arguments) {
        m_positional_arguments.push_back(std::move(argument));
    }
}

std::vector<Expression *> Call::positional_arguments() const {
    std::vector<Expression *> expressions;
    for (auto &expression : m_positional_arguments) {
        expressions.push_back(expression.get());
    }
    return expressions;
}

std::vector<types::Type *> Call::positional_argument_types() const {
    std::vector<types::Type *> types;
    for (auto &expression : m_positional_arguments) {
        types.push_back(expression->type());
    }
    return types;
}

std::map<std::string, Expression *> Call::keyword_arguments() const {
    std::map<std::string, Expression *> expressions;
    for (auto &entry : m_keyword_arguments) {
        expressions[entry.first] = entry.second.get();
    }
    return expressions;
}

std::map<std::string, types::Type *> Call::keyword_argument_types() const {
    std::map<std::string, types::Type *> types;
    for (auto &entry : m_keyword_arguments) {
        types[entry.first] = entry.second->type();
    }
    return types;
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

CCall::CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameters, std::unique_ptr<Name> given_return_type, std::vector<std::unique_ptr<Expression>> arguments) : Expression(token), m_name(std::move(name)), m_given_return_type(std::move(given_return_type)) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }

    for (auto &argument : arguments) {
        m_arguments.push_back(std::move(argument));
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

void CCall::accept(Visitor *visitor) {
    visitor->visit(this);
}

Cast::Cast(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Name> new_type) : Expression(token), m_operand(std::move(operand)), m_new_type(std::move(new_type)) {

}

void Cast::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Token token, std::unique_ptr<VariableDeclaration> lhs, std::unique_ptr<Expression> rhs) : Expression(token), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {

}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

Selector::Selector(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Name> field) : Expression(token), m_operand(std::move(operand)), m_field(std::move(field)) {

}

Selector::Selector(Token token, std::unique_ptr<Expression> operand, std::string field) : Selector(token, std::move(operand), std::make_unique<Name>(token, field)) {

}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

While::While(Token token, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> body) : Expression(token), m_condition(std::move(condition)), m_body(std::move(body)) {

}

void While::accept(Visitor *visitor) {
    visitor->visit(this);
}

If::If(Token token, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> true_case, std::unique_ptr<Expression> false_case) : Expression(token), m_condition(std::move(condition)), m_true_case(std::move(true_case)), m_false_case(std::move(false_case)) {

}

void If::accept(Visitor *visitor) {
    visitor->visit(this);
}

Return::Return(Token token, std::unique_ptr<Expression> expression) : Expression(token), m_expression(std::move(expression)) {

}

void Return::accept(Visitor *visitor) {
    visitor->visit(this);
}

Spawn::Spawn(Token token, std::unique_ptr<Call> call) : Expression(token), m_call(std::move(call)) {

}

void Spawn::accept(Visitor *visitor) {
    visitor->visit(this);
}

Case::Case(Token token, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> assignment, std::unique_ptr<Expression> body) :
        Expression(token), m_condition(std::move(condition)), m_assignment(std::move(assignment)), m_body(std::move(body))
{

}

void Case::accept(Visitor *visitor) {
    // visitor->visit(this);
}

Switch::Switch(Token token, std::unique_ptr<Expression> expression, std::vector<std::unique_ptr<Case>> cases, std::unique_ptr<Expression> default_case) : Expression(token), m_expression(std::move(expression)), m_default_case(std::move(default_case)) {
    for (auto &entry : cases) {
        m_cases.push_back(std::move(entry));
    }
}

std::vector<Case *> Switch::cases() const {
    std::vector<Case *> cases;
    for (auto &entry : m_cases) {
        cases.push_back(entry.get());
    }
    return cases;
}

void Switch::accept(Visitor *visitor) {
    visitor->visit(this);
}

Let::Let(Token token, std::unique_ptr<Assignment> assignment, std::unique_ptr<Expression> body) : Expression(token), m_assignment(std::move(assignment)), m_body(std::move(body)) {

}

Let::Let(Token token, std::string name, std::unique_ptr<Expression> value, std::unique_ptr<Expression> body) : Expression(token), m_body(std::move(body)) {
    auto name_node = std::make_unique<Name>(token, name);

    auto variable_declaration = std::make_unique<VariableDeclaration>(
        token, std::move(name_node), nullptr, false
    );

    m_assignment = std::make_unique<Assignment>(
        token, std::move(variable_declaration), std::move(value)
    );
}

void Let::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<Name> given_type) : Expression(token), m_inout(inout), m_name(std::move(name)), m_given_type(std::move(given_type)) {

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

Def::Def(Token token, std::unique_ptr<Expression> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Expression> body, std::unique_ptr<Name> given_return_type) : Expression(token), m_name(std::move(name)), m_builtin(builtin), m_body(std::move(body)), m_given_return_type(std::move(given_return_type)) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }
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

Parameter *Def::parameter(size_t index) const {
    return m_parameters[index].get();
}

size_t Def::no_parameters() const {
    return m_parameters.size();
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

Type::Type(Token token, std::unique_ptr<Name> name, bool builtin) : Expression(token), m_name(std::move(name)), m_builtin(builtin) {

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

void Type::add_field(std::unique_ptr<Name> name, std::unique_ptr<Name> type) {
    m_field_names.push_back(std::move(name));
    m_field_types.push_back(std::move(type));
}

std::vector<Name *> Type::field_names() const {
    std::vector<Name *> field_names;
    for (auto &name : m_field_names) {
        field_names.push_back(name.get());
    }
    return field_names;
}

std::vector<Name *> Type::field_types() const {
    std::vector<Name *> field_types;
    for (auto &type : m_field_types) {
        field_types.push_back(type.get());
    }
    return field_types;
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

MethodSignature::MethodSignature(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameter_types, std::unique_ptr<Name> return_type) : Node(token), m_name(std::move(name)), m_return_type(std::move(return_type)) {
    for (auto &parameter : parameter_types) {
        m_parameter_types.push_back(std::move(parameter));
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

Module::Module(Token token, std::unique_ptr<Name> name, std::unique_ptr<Block> body) : Expression(token), m_name(std::move(name)), m_body(std::move(body)) {

}

void Module::accept(Visitor *visitor) {
    visitor->visit(this);
}

Import::Import(Token token, std::unique_ptr<String> path) : Expression(token), m_path(std::move(path)) {

}

void Import::accept(Visitor *visitor) {
    visitor->visit(this);
}

SourceFile::SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code) : Expression(token), m_name(name), m_code(std::move(code)) {
    for (auto &import : imports) {
        m_imports.push_back(std::move(import));
    }
}

std::vector<SourceFile *> SourceFile::imports() const {
    std::vector<SourceFile *> imports;
    for (auto &import : m_imports) {
        imports.push_back(import.get());
    }
    return imports;
}

void SourceFile::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor()
{

}
