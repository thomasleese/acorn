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

Node::Node(NodeKind kind, Token token) : m_kind(std::move(kind)), m_token(std::move(token)), m_type(nullptr) {

}

std::string Node::to_string() const {
    std::stringstream ss;
    ss << kind_string() << "(" << m_token.to_string() << ")";
    return ss.str();
}

std::string Node::kind_string() const {
    switch (m_kind) {
    case NK_Block:
        return "Block";
    case NK_Name:
        return "Name";
    case NK_Selector:
        return "Selector";
    case NK_TypeName:
        return "TypeName";
    case NK_DeclName:
        return "DeclName";
    case NK_ParamName:
        return "ParamName";
    case NK_DeclHolder:
        return "DeclHolder";
    case NK_VarDecl:
        return "VarDecl";
    case NK_Int:
        return "Int";
    case NK_Float:
        return "Float";
    case NK_Complex:
        return "Complex";
    case NK_String:
        return "String";
    case NK_List:
        return "List";
    case NK_Tuple:
        return "Tuple";
    case NK_Dictionary:
        return "Dictionary";
    case NK_Call:
        return "Call";
    case NK_CCall:
        return "CCall";
    case NK_Cast:
        return "Cast";
    case NK_Assignment:
        return "Assignment";
    case NK_While:
        return "While";
    case NK_If:
        return "If";
    case NK_Return:
        return "Return";
    case NK_Spawn:
        return "Spawn";
    case NK_Case:
        return "Case";
    case NK_Switch:
        return "Switch";
    case NK_Let:
        return "Let";
    case NK_Parameter:
        return "Parameter";
    case NK_DefDecl:
        return "DefDecl";
    case NK_TypeDecl:
        return "TypeDecl";
    case NK_ModuleDecl:
        return "ModuleDecl";
    case NK_Import:
        return "Import";
    case NK_SourceFile:
        return "SourceFile";
    }
}

void Node::copy_type_from(Node *node) {
    set_type(node->type());
}

bool Node::has_compatible_type_with(Node *node) const {
    // TODO set a warning here if the type is null?
    return m_type->is_compatible(node->type());
}

std::string Node::type_name() const {
    if (m_type == nullptr) {
        return "null";
    }

    return m_type->name();
}

Block::Block(Token token, std::vector<std::unique_ptr<Node>> expressions)
    : Node(NK_Block, token), m_expressions(std::move(expressions)) {

}

Block::Block(Token token, std::unique_ptr<Node> expression) : Node(NK_Block, token) {
    m_expressions.push_back(std::move(expression));
}

Block *Block::clone() const {
    std::vector<std::unique_ptr<Node>> cloned_expressions;
    for (auto &expression : m_expressions) {
        cloned_expressions.push_back(std::unique_ptr<Node>(expression->clone()));
    }

    return new Block(token(), std::move(cloned_expressions));
}

Name::Name(Token token, std::string value) : Node(NK_Name, token), m_value(value) {

}

Name *Name::clone() const {
    return new Name(token(), m_value);
}

TypeName::TypeName(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters)
    : Node(NK_TypeName, token), m_name(std::move(name)), m_parameters(std::move(parameters)) {

}

DeclName::DeclName(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameters)
    : Node(NK_DeclName, token), m_name(std::move(name)), m_parameters(std::move(parameters)) {

}

DeclName::DeclName(Token token, std::unique_ptr<Name> name) : Node(NK_DeclName, token), m_name(std::move(name)) {

}

DeclName::DeclName(Token token, std::string name) : DeclName(token, std::make_unique<Name>(token, name)) {

}

void DeclName::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

ParamName::ParamName(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters) :
    Node(NK_ParamName, token), m_name(std::move(name)), m_parameters(std::move(parameters)) {

}

ParamName::ParamName(Token token, std::unique_ptr<Name> name) :
    Node(NK_ParamName, token), m_name(std::move(name)) {

}

ParamName::ParamName(Token token, std::string name) :
    ParamName(token, std::make_unique<Name>(token, name)) {

}

DeclNode::DeclNode(NodeKind kind, Token token, bool builtin, std::unique_ptr<DeclName> name) : Node(kind, token), m_builtin(builtin), m_name(std::move(name)) {

}

DeclNode::DeclNode(NodeKind kind, Token token, bool builtin, std::unique_ptr<Name> name) : DeclNode(kind, token, builtin, std::make_unique<DeclName>(token, std::move(name))) {

}

DeclNode::DeclNode(NodeKind kind, Token token, bool builtin, std::string name) : DeclNode(kind, token, builtin, std::make_unique<Name>(token, name)) {

}

void DeclNode::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

DeclHolder::DeclHolder(Token token, std::unique_ptr<DeclNode> main_instance) : Node(NK_DeclHolder, token) {
    main_instance->set_holder(this);
    m_instances.push_back(std::move(main_instance));
}

void DeclHolder::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_instances[0]->set_type(type);
}

VarDecl::VarDecl(Token token, std::unique_ptr<DeclName> name, std::unique_ptr<TypeName> type, bool builtin) : DeclNode(NK_VarDecl, token, builtin, std::move(name)), m_given_type(std::move(type)) {

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

CCall::CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters, std::unique_ptr<TypeName> return_type, std::vector<std::unique_ptr<Node>> arguments) : Node(NK_CCall, token), m_name(std::move(name)), m_return_type(std::move(return_type)) {
    for (auto &parameter : parameters) {
        m_parameters.push_back(std::move(parameter));
    }

    for (auto &argument : arguments) {
        m_arguments.push_back(std::move(argument));
    }
}

Cast::Cast(Token token, std::unique_ptr<Node> operand, std::unique_ptr<TypeName> new_type) : Node(NK_Cast, token), m_operand(std::move(operand)), m_new_type(std::move(new_type)) {

}

Assignment::Assignment(Token token, std::unique_ptr<DeclHolder> lhs, std::unique_ptr<Node> rhs) : Node(NK_Assignment, token), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {

}

Selector::Selector(Token token, std::unique_ptr<Node> operand, std::unique_ptr<ParamName> field) : Node(NK_Selector, token), m_operand(std::move(operand)), m_field(std::move(field)) {

}

Selector::Selector(Token token, std::unique_ptr<Node> operand, std::string field) : Selector(token, std::move(operand), std::make_unique<ParamName>(token, field)) {

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

Let::Let(Token token, std::unique_ptr<Assignment> assignment) : Node(NK_Let, token), m_assignment(std::move(assignment)) {

}

Let::Let(Token token, std::string name, std::unique_ptr<Node> value) : Node(NK_Let, token) {
    auto name_node = std::make_unique<DeclName>(token, name);

    auto var_decl = std::make_unique<VarDecl>(
        token, std::move(name_node), nullptr, false
    );

    auto var = std::make_unique<DeclHolder>(
        var_decl->token(), std::move(var_decl)
    );

    m_assignment = std::make_unique<Assignment>(
        token, std::move(var), std::move(value)
    );
}

Parameter::Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<TypeName> given_type) : Node(NK_Parameter, token), m_inout(inout), m_name(std::move(name)), m_given_type(std::move(given_type)) {

}

DefDecl::DefDecl(Token token, std::unique_ptr<DeclName> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Node> body, std::unique_ptr<TypeName> return_type) : DeclNode(NK_DefDecl, token, builtin, std::move(name)), m_parameters(std::move(parameters)), m_return_type(std::move(return_type)), m_body(std::move(body)) {

}

TypeDecl::TypeDecl(Token token, std::unique_ptr<DeclName> name) : DeclNode(NK_TypeDecl, token, true, std::move(name)) {

}

TypeDecl::TypeDecl(Token token, std::unique_ptr<DeclName> name, std::unique_ptr<TypeName> alias) : DeclNode(NK_TypeDecl, token, false, std::move(name)), m_alias(std::move(alias)) {

}

TypeDecl::TypeDecl(Token token, std::unique_ptr<DeclName> name, std::vector<std::unique_ptr<Name>> field_names, std::vector<std::unique_ptr<TypeName>> field_types) : DeclNode(NK_TypeDecl, token, false, std::move(name)) {
    for (auto &field_name : field_names) {
        m_field_names.push_back(std::move(field_name));
    }

    for (auto &field_type : field_types) {
        m_field_types.push_back(std::move(field_type));
    }
}

ModuleDecl::ModuleDecl(Token token, std::unique_ptr<DeclName> name, std::unique_ptr<Block> body) : DeclNode(NK_ModuleDecl, token, false, std::move(name)), m_body(std::move(body)) {

}

Import::Import(Token token, std::unique_ptr<String> path) : Node(NK_Import, token), m_path(std::move(path)) {

}

SourceFile::SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code) : Node(NK_SourceFile, token), m_name(name), m_code(std::move(code)) {
    for (auto &import : imports) {
        m_imports.push_back(std::move(import));
    }
}
