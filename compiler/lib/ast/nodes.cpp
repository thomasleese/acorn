//
// Created by Thomas Leese on 12/01/2017.
//

#include <iostream>
#include <sstream>

#include "acorn/typesystem/types.h"
#include "acorn/ast/visitor.h"

#include "acorn/ast/nodes.h"

using namespace acorn;
using namespace acorn::ast;

Node::Node(NodeKind kind, Token token) : m_kind(kind), m_token(token), m_type(nullptr) {

}

Node::~Node() {

}

void Node::copy_type_from(Node *node) {
    set_type(node->type());
}

bool Node::has_compatible_type_with(Node *node) const {
    // TODO set a warning here if the type is null?
    return m_type->is_compatible(node->type());
}

std::string Node::type_name() const {
    if (m_type) {
        return m_type->name();
    } else {
        return "null";
    }
}

Block::Block(Token token, std::vector<std::unique_ptr<Node>> expressions) : Node(NK_Block, token) {
    for (auto &expression : expressions) {
        m_expressions.push_back(std::move(expression));
    }
}

Name::Name(Token token, std::string value) : Node(NK_Name, token), m_value(value) {

}

Name::Name(Token token, std::string value, std::vector<std::unique_ptr<Name>> parameters) : Name(token, value) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }
}

VariableDeclaration::VariableDeclaration(Token token, std::unique_ptr<Name> name, std::unique_ptr<Name> type, bool builtin) : Node(NK_VariableDeclaration, token), m_name(std::move(name)), m_given_type(std::move(type)), m_builtin(builtin) {

}

void VariableDeclaration::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

Int::Int(Token token, std::string value) : Node(NK_Int, token), m_value(value) {

}

Float::Float(Token token, std::string value) : Node(NK_Float, token), m_value(value) {

}

String::String(Token token, std::string value) : Node(NK_String, token), m_value(value) {

}

Sequence::Sequence(NodeKind kind, Token token, std::vector<std::unique_ptr<Node>> elements) : Node(kind, token) {
    for (auto &element : elements) {
        m_elements.push_back(std::move(element));
    }
}

List::List(Token token, std::vector<std::unique_ptr<Node>> elements) : Sequence(NK_List, token, std::move(elements)) {

}

Tuple::Tuple(Token token, std::vector<std::unique_ptr<Node>> elements) : Sequence(NK_Tuple, token, std::move(elements)) {

}

Dictionary::Dictionary(Token token, std::vector<std::unique_ptr<Node>> keys, std::vector<std::unique_ptr<Node>> values) : Node(NK_Dictionary, token) {
    assert(keys.size() == values.size());

    for (auto &key : keys) {
        m_keys.push_back(std::move(key));
    }

    for (auto &value : values) {
        m_values.push_back(std::move(value));
    }
}

Call::Call(Token token, std::unique_ptr<Node> operand, std::vector<std::unique_ptr<Node>> positional_arguments, std::map<std::string, std::unique_ptr<Node>> keyword_arguments) : Node(NK_Call, token), m_operand(std::move(operand)), m_method_index(0), m_method_specialisation_index(0) {
    for (auto &argument : positional_arguments) {
        m_positional_arguments.push_back(std::move(argument));
    }

    for (auto &entry : keyword_arguments) {
        m_keyword_arguments[entry.first] = std::move(entry.second);
    }
}

Call::Call(Token token, std::unique_ptr<Node> operand, std::unique_ptr<Node> arg1, std::unique_ptr<Node> arg2) : Node(NK_Call, token), m_operand(std::move(operand)), m_method_index(0), m_method_specialisation_index(0) {
    if (arg1) {
        m_positional_arguments.push_back(std::move(arg1));
    }

    if (arg2) {
        m_positional_arguments.push_back(std::move(arg2));
    }
}

Call::Call(Token token, std::string name, std::unique_ptr<Node> arg1, std::unique_ptr<Node> arg2) : Call(token, std::make_unique<Name>(token, name), std::move(arg1), std::move(arg2)) {

}

Call::Call(Token token, std::string name, std::vector<std::unique_ptr<Node>> arguments) : Call(token, name) {
    for (auto &argument : arguments) {
        m_positional_arguments.push_back(std::move(argument));
    }
}

std::vector<typesystem::Type *> Call::positional_argument_types() const {
    std::vector<typesystem::Type *> types;
    for (auto &expression : m_positional_arguments) {
        types.push_back(expression->type());
    }
    return types;
}

std::map<std::string, typesystem::Type *> Call::keyword_argument_types() const {
    std::map<std::string, typesystem::Type *> types;
    for (auto &entry : m_keyword_arguments) {
        types[entry.first] = entry.second->type();
    }
    return types;
}

CCall::CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameters, std::unique_ptr<Name> given_return_type, std::vector<std::unique_ptr<Node>> arguments) : Node(NK_CCall, token), m_name(std::move(name)), m_given_return_type(std::move(given_return_type)) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }

    for (auto &argument : arguments) {
        m_arguments.push_back(std::move(argument));
    }
}

Cast::Cast(Token token, std::unique_ptr<Node> operand, std::unique_ptr<Name> new_type) : Node(NK_Cast, token), m_operand(std::move(operand)), m_new_type(std::move(new_type)) {

}

Assignment::Assignment(Token token, std::unique_ptr<VariableDeclaration> lhs, std::unique_ptr<Node> rhs) : Node(NK_Assignment, token), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {

}

Selector::Selector(Token token, std::unique_ptr<Node> operand, std::unique_ptr<Name> field) : Node(NK_Selector, token), m_operand(std::move(operand)), m_field(std::move(field)) {

}

Selector::Selector(Token token, std::unique_ptr<Node> operand, std::string field) : Selector(token, std::move(operand), std::make_unique<Name>(token, field)) {

}

While::While(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> body) : Node(NK_While, token), m_condition(std::move(condition)), m_body(std::move(body)) {

}

If::If(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> true_case, std::unique_ptr<Node> false_case) : Node(NK_If, token), m_condition(std::move(condition)), m_true_case(std::move(true_case)), m_false_case(std::move(false_case)) {

}

Return::Return(Token token, std::unique_ptr<Node> expression) : Node(NK_Return, token), m_expression(std::move(expression)) {

}

Spawn::Spawn(Token token, std::unique_ptr<Call> call) : Node(NK_Spawn, token), m_call(std::move(call)) {

}

Case::Case(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> assignment, std::unique_ptr<Node> body) :
        Node(NK_Case, token), m_condition(std::move(condition)), m_assignment(std::move(assignment)), m_body(std::move(body))
{

}

Switch::Switch(Token token, std::unique_ptr<Node> expression, std::vector<std::unique_ptr<Case>> cases, std::unique_ptr<Node> default_case) : Node(NK_Switch, token), m_expression(std::move(expression)), m_default_case(std::move(default_case)) {
    for (auto &entry : cases) {
        m_cases.push_back(std::move(entry));
    }
}

Let::Let(Token token, std::unique_ptr<Assignment> assignment, std::unique_ptr<Node> body) : Node(NK_Let, token), m_assignment(std::move(assignment)), m_body(std::move(body)) {

}

Let::Let(Token token, std::string name, std::unique_ptr<Node> value, std::unique_ptr<Node> body) : Node(NK_Let, token), m_body(std::move(body)) {
    auto name_node = std::make_unique<Name>(token, name);

    auto variable_declaration = std::make_unique<VariableDeclaration>(
        token, std::move(name_node), nullptr, false
    );

    m_assignment = std::make_unique<Assignment>(
        token, std::move(variable_declaration), std::move(value)
    );
}

Parameter::Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<Name> given_type) : Node(NK_Parameter, token), m_inout(inout), m_name(std::move(name)), m_given_type(std::move(given_type)) {

}

DefInstance::DefInstance(Token token, std::unique_ptr<Selector> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Node> body, std::unique_ptr<Name> given_return_type) : Node(NK_DefInstance, token), m_builtin(builtin), m_name(std::move(name)), m_parameters(std::move(parameters)), m_given_return_type(std::move(given_return_type)), m_body(std::move(body)) {

}

void DefInstance::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

Def::Def(Token token, std::unique_ptr<DefInstance> main_instance) : Node(NK_DefInstance, token) {
    m_instances.push_back(std::move(main_instance));
}

Type::Type(Token token, std::unique_ptr<Name> name) : Node(NK_Type, token), m_name(std::move(name)), m_builtin(true) {

}

Type::Type(Token token, std::unique_ptr<Name> name, std::unique_ptr<Name> alias) : Node(NK_Type, token), m_name(std::move(name)), m_builtin(false), m_alias(std::move(alias)) {

}

Type::Type(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> field_names, std::vector<std::unique_ptr<Name>> field_types) : Node(NK_Type, token), m_name(std::move(name)), m_builtin(false) {
    for (auto &field_name : field_names) {
        m_field_names.push_back(std::move(field_name));
    }

    for (auto &field_type : field_types) {
        m_field_types.push_back(std::move(field_type));
    }
}

void Type::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

Module::Module(Token token, std::unique_ptr<Name> name, std::unique_ptr<Block> body) : Node(NK_Module, token), m_name(std::move(name)), m_body(std::move(body)) {

}

Import::Import(Token token, std::unique_ptr<String> path) : Node(NK_Import, token), m_path(std::move(path)) {

}

SourceFile::SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code) : Node(NK_SourceFile, token), m_name(name), m_code(std::move(code)) {
    for (auto &import : imports) {
        m_imports.push_back(std::move(import));
    }
}
