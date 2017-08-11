//
// Created by Thomas Leese on 12/01/2017.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "acorn/typesystem/types.h"
#include "acorn/ast/visitor.h"

#include "acorn/ast/nodes.h"

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

void VariableDeclaration::set_type(typesystem::Type *type) {
    Expression::set_type(type);
    m_name->set_type(type);
}

void VariableDeclaration::accept(Visitor *visitor) {
    visitor->visit(this);
}

Int::Int(Token token, std::string value) : Expression(token), m_value(value) {

}

void Int::accept(Visitor *visitor) {
    visitor->visit(this);
}

Float::Float(Token token, std::string value) : Expression(token), m_value(value) {

}

void Float::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Complex::accept(Visitor *visitor) {
    visitor->visit(this);
}

String::String(Token token, std::string value) : Expression(token), m_value(value) {

}

void String::accept(Visitor *visitor) {
    visitor->visit(this);
}

List::List(Token token, std::vector<std::unique_ptr<Expression>> elements) : Expression(token) {
    for (auto &element : elements) {
        m_elements.push_back(std::move(element));
    }
}

std::vector<Expression *> List::elements() const {
    std::vector<Expression *> elements;
    for (auto &element : m_elements) {
        elements.push_back(element.get());
    }
    return elements;
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

void Dictionary::accept(Visitor *visitor) {
    visitor->visit(this);
}

Tuple::Tuple(Token token, std::vector<std::unique_ptr<Expression>> elements) : Expression(token) {
    for (auto &element : elements) {
        m_elements.push_back(std::move(element));
    }
}

std::vector<Expression *> Tuple::elements() const {
    std::vector<Expression *> elements;
    for (auto &element : m_elements) {
        elements.push_back(element.get());
    }
    return elements;
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

std::vector<typesystem::Type *> Call::positional_argument_types() const {
    std::vector<typesystem::Type *> types;
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

std::map<std::string, typesystem::Type *> Call::keyword_argument_types() const {
    std::map<std::string, typesystem::Type *> types;
    for (auto &entry : m_keyword_arguments) {
        types[entry.first] = entry.second->type();
    }
    return types;
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

std::vector<Name *> CCall::parameters() const {
    std::vector<Name *> parameters;
    for (auto &parameter : m_parameters) {
        parameters.push_back(parameter.get());
    }
    return parameters;
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

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

MethodSignature::MethodSignature(Token token, std::unique_ptr<Selector> name, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Name> given_return_type) : Node(token), m_name(std::move(name)), m_given_return_type(std::move(given_return_type)) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }
}

std::vector<Parameter *> MethodSignature::parameters() const {
    std::vector<Parameter *> parameters;
    for (auto &parameter : m_parameters) {
        parameters.push_back(parameter.get());
    }
    return parameters;
}

void MethodSignature::accept(Visitor *visitor) {

}

Def::Def(Token token, std::unique_ptr<Selector> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Expression> body, std::unique_ptr<Name> given_return_type) : Expression(token), m_builtin(builtin), m_body(std::move(body)) {
    m_signature = std::make_unique<MethodSignature>(token, std::move(name), std::move(parameters), std::move(given_return_type));
}

void Def::set_type(typesystem::Type *type) {
    Expression::set_type(type);
    m_signature->name()->set_type(type);
}

void Def::accept(Visitor *visitor) {
    visitor->visit(this);
}

Type::Type(Token token, std::unique_ptr<Name> name) : Expression(token), m_name(std::move(name)), m_builtin(true) {

}

Type::Type(Token token, std::unique_ptr<Name> name, std::unique_ptr<Name> alias) : Expression(token), m_name(std::move(name)), m_builtin(false), m_alias(std::move(alias)) {

}

Type::Type(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> field_names, std::vector<std::unique_ptr<Name>> field_types) : Expression(token), m_name(std::move(name)), m_builtin(false) {
    for (auto &field_name : field_names) {
        m_field_names.push_back(std::move(field_name));
    }

    for (auto &field_type : field_types) {
        m_field_types.push_back(std::move(field_type));
    }
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

void Type::set_type(typesystem::Type *type) {
    Expression::set_type(type);
    m_name->set_type(type);
}

void Type::accept(Visitor *visitor) {
    visitor->visit(this);
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
