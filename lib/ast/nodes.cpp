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

using std::make_unique;
using std::unique_ptr;

Node::Node(NodeKind kind, Token token)
    : m_kind(std::move(kind)), m_token(std::move(token)), m_type(nullptr) { }

std::string Node::to_string() const {
    std::stringstream ss;
    ss << kind_string() << "(" << m_token << ")";
    return ss.str();
}

const char *Node::kind_string() const {
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
    case NK_SpecialisedDecl:
        return "SpecialisedDecl";
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

Name::Name(Token token, std::string value)
    : Node(NK_Name, token), m_value(value) { }

Name *Name::clone() const {
    return new Name(token(), m_value);
}

TypeName::TypeName(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters)
    : Node(NK_TypeName, token), m_name(std::move(name)), m_parameters(std::move(parameters)) {

}

TypeName::TypeName(Token token, std::unique_ptr<Name> name)
    : TypeName(token, std::move(name), std::vector<std::unique_ptr<TypeName>>()) { }

TypeName *TypeName::clone() const {
    auto cloned_name = unique_ptr<Name>(m_name->clone());

    std::vector<std::unique_ptr<TypeName>> cloned_parameters;
    for (auto &parameter : m_parameters) {
        cloned_parameters.push_back(unique_ptr<TypeName>(parameter->clone()));
    }

    return new TypeName(token(), std::move(cloned_name), std::move(cloned_parameters));
}

DeclName::DeclName(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameters)
    : Node(NK_DeclName, token), m_name(std::move(name)), m_parameters(std::move(parameters)) { }

DeclName::DeclName(Token token, std::unique_ptr<Name> name)
    : DeclName(token, std::move(name), std::vector<unique_ptr<Name>>()) { }

DeclName::DeclName(Token token, std::string name)
    : DeclName(token, std::make_unique<Name>(token, name)) { }

DeclName *DeclName::clone() const {
    auto cloned_name = unique_ptr<Name>(m_name->clone());

    std::vector<std::unique_ptr<Name>> cloned_parameters;
    for (auto &parameter : m_parameters) {
        cloned_parameters.push_back(unique_ptr<Name>(parameter->clone()));
    }

    return new DeclName(token(), std::move(cloned_name), std::move(cloned_parameters));
}

void DeclName::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

ParamName::ParamName(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters)
    : Node(NK_ParamName, token), m_name(std::move(name)), m_parameters(std::move(parameters)) { }

ParamName::ParamName(Token token, std::unique_ptr<Name> name)
    : Node(NK_ParamName, token), m_name(std::move(name)) { }

ParamName::ParamName(Token token, std::string name)
    : ParamName(token, std::make_unique<Name>(token, name)) { }

ParamName *ParamName::clone() const {
    auto cloned_name = unique_ptr<Name>(m_name->clone());

    std::vector<std::unique_ptr<TypeName>> cloned_parameters;
    for (auto &parameter : m_parameters) {
        cloned_parameters.push_back(unique_ptr<TypeName>(parameter->clone()));
    }

    return new ParamName(token(), std::move(cloned_name), std::move(cloned_parameters));
}

DeclNode::DeclNode(NodeKind kind, Token token, bool builtin, std::unique_ptr<DeclName> name)
    : Node(kind, token), m_builtin(builtin), m_name(std::move(name)) { }

DeclNode::DeclNode(NodeKind kind, Token token, bool builtin, std::unique_ptr<Name> name)
    : DeclNode(kind, token, builtin, std::make_unique<DeclName>(token, std::move(name))) { }

DeclNode::DeclNode(NodeKind kind, Token token, bool builtin, std::string name)
    : DeclNode(kind, token, builtin, std::make_unique<Name>(token, name)) { }

void DeclNode::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_name->set_type(type);
}

SpecialisedDecl::SpecialisedDecl(Token token, std::unique_ptr<DeclNode> declaration)
    : Node (NK_SpecialisedDecl, token), m_declaration(std::move(declaration)) { }

SpecialisedDecl *SpecialisedDecl::clone() const {
    auto cloned_declaration = unique_ptr<DeclNode>(m_declaration->clone());

    return new SpecialisedDecl(token(), std::move(cloned_declaration));
}

DeclHolder::DeclHolder(Token token, std::unique_ptr<DeclNode> main_instance, std::vector<unique_ptr<SpecialisedDecl>> specialised_instances)
    : Node(NK_DeclHolder, token), m_main_instance(std::move(main_instance)), m_specialised_instances(std::move(specialised_instances)) {
    m_main_instance->set_holder(this);

    for (auto &instance : m_specialised_instances) {
        instance->set_holder(this);
    }
}

DeclHolder::DeclHolder(Token token, std::unique_ptr<DeclNode> main_instance)
    : DeclHolder(token, std::move(main_instance), std::vector<unique_ptr<SpecialisedDecl>>()) { }

DeclHolder *DeclHolder::clone() const {
    auto cloned_main_instance = unique_ptr<DeclNode>(m_main_instance->clone());

    std::vector<std::unique_ptr<SpecialisedDecl>> cloned_specialised_instances;
    for (auto &instance : m_specialised_instances) {
        cloned_specialised_instances.push_back(unique_ptr<SpecialisedDecl>(instance->clone()));
    }

    return new DeclHolder(token(), std::move(cloned_main_instance), std::move(cloned_specialised_instances));
}

void DeclHolder::create_specialisation() {
    auto cloned_decl = unique_ptr<DeclNode>(m_main_instance->clone());

    auto specialised_decl = make_unique<SpecialisedDecl>(token(), std::move(cloned_decl));

    m_specialised_instances.push_back(std::move(specialised_decl));
}

void DeclHolder::set_type(typesystem::Type *type) {
    Node::set_type(type);
    m_main_instance->set_type(type);
}

VarDecl::VarDecl(Token token, std::unique_ptr<DeclName> name, std::unique_ptr<TypeName> type, bool builtin)
    : DeclNode(NK_VarDecl, token, builtin, std::move(name)), m_given_type(std::move(type)) { }

VarDecl *VarDecl::clone() const {
    auto cloned_name = unique_ptr<DeclName>(m_name->clone());

    unique_ptr<TypeName> cloned_type;
    if (m_given_type) {
        cloned_type = unique_ptr<TypeName>(m_given_type->clone());
    }

    return new VarDecl(token(), std::move(cloned_name), std::move(cloned_type), m_builtin);
}

Int::Int(Token token, std::string value)
    : Node(NK_Int, token), m_value(value) { }

Int *Int::clone() const {
    return new Int(token(), m_value);
}

Float::Float(Token token, std::string value)
    : Node(NK_Float, token), m_value(value) { }

Float *Float::clone() const {
    return new Float(token(), m_value);
}

Complex::Complex(Token token) : Node(NK_Complex, token) { }

Complex *Complex::clone() const {
    return new Complex(token());
}

String::String(Token token, std::string value)
    : Node(NK_String, token), m_value(value) { }

String *String::clone() const {
    return new String(token(), m_value);
}

Sequence::Sequence(NodeKind kind, Token token, std::vector<std::unique_ptr<Node>> elements) : Node(kind, token) {
    for (auto &element : elements) {
        m_elements.push_back(std::move(element));
    }
}

List::List(Token token, std::vector<std::unique_ptr<Node>> elements)
    : Sequence(NK_List, token, std::move(elements)) { }

List *List::clone() const {
    std::vector<unique_ptr<Node>> cloned_elements;
    for (auto &element : m_elements) {
        cloned_elements.push_back(unique_ptr<Node>(element->clone()));
    }

    return new List(token(), std::move(cloned_elements));
}

Tuple::Tuple(Token token, std::vector<std::unique_ptr<Node>> elements)
    : Sequence(NK_Tuple, token, std::move(elements)) { }

Tuple *Tuple::clone() const {
    std::vector<unique_ptr<Node>> cloned_elements;
    for (auto &element : m_elements) {
        cloned_elements.push_back(unique_ptr<Node>(element->clone()));
    }

    return new Tuple(token(), std::move(cloned_elements));
}

Dictionary::Dictionary(Token token, std::vector<std::unique_ptr<Node>> keys, std::vector<std::unique_ptr<Node>> values)
    : Node(NK_Dictionary, token), m_keys(std::move(keys)), m_values(std::move(values)) {
    assert(keys.size() == values.size());
}

Dictionary* Dictionary::clone() const {
    std::vector<unique_ptr<Node>> cloned_keys;
    for (auto &key : m_keys) {
        cloned_keys.push_back(unique_ptr<Node>(key->clone()));
    }

    std::vector<unique_ptr<Node>> cloned_values;
    for (auto &value : m_values) {
        cloned_values.push_back(unique_ptr<Node>(value->clone()));
    }

    return new Dictionary(token(), std::move(cloned_keys), std::move(cloned_values));
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

Call::Call(Token token, std::string name, std::unique_ptr<Node> arg1, std::unique_ptr<Node> arg2)
    : Call(token, std::make_unique<Name>(token, name), std::move(arg1), std::move(arg2)) { }

Call::Call(Token token, std::string name, std::vector<std::unique_ptr<Node>> arguments) : Call(token, name) {
    for (auto &argument : arguments) {
        m_positional_arguments.push_back(std::move(argument));
    }
}

Call *Call::clone() const {
    auto cloned_operand = unique_ptr<Node>(m_operand->clone());

    std::vector<unique_ptr<Node>> cloned_positional_arguments;
    for (auto &argument : m_positional_arguments) {
        cloned_positional_arguments.push_back(unique_ptr<Node>(argument->clone()));
    }

    std::map<std::string, unique_ptr<Node>> cloned_keyword_arguments;
    for (auto &entry : m_keyword_arguments) {
        cloned_keyword_arguments[entry.first] = unique_ptr<Node>(entry.second->clone());
    }

    return new Call(token(), std::move(cloned_operand), std::move(cloned_positional_arguments), std::move(cloned_keyword_arguments));
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

CCall::CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters, std::unique_ptr<TypeName> return_type, std::vector<std::unique_ptr<Node>> arguments)
    : Node(NK_CCall, token), m_name(std::move(name)), m_parameters(std::move(parameters)), m_return_type(std::move(return_type)), m_arguments(std::move(arguments)) { }

CCall *CCall::clone() const {
    auto cloned_name = unique_ptr<Name>(m_name->clone());

    std::vector<unique_ptr<TypeName>> cloned_parameters;
    for (auto &parameter : m_parameters) {
        cloned_parameters.push_back(unique_ptr<TypeName>(parameter->clone()));
    }

    auto cloned_return_type = unique_ptr<TypeName>(m_return_type->clone());

    std::vector<unique_ptr<Node>> cloned_arguments;
    for (auto &argument : m_arguments) {
        cloned_arguments.push_back(unique_ptr<Node>(argument->clone()));
    }

    return new CCall(token(), std::move(cloned_name), std::move(cloned_parameters), std::move(cloned_return_type), std::move(cloned_arguments));
}

Cast::Cast(Token token, std::unique_ptr<Node> operand, std::unique_ptr<TypeName> new_type)
    : Node(NK_Cast, token), m_operand(std::move(operand)), m_new_type(std::move(new_type)) { }

Cast *Cast::clone() const {
    auto cloned_operand = unique_ptr<Node>(m_operand->clone());

    auto cloned_new_type = unique_ptr<TypeName>(m_new_type->clone());

    return new Cast(token(), std::move(cloned_operand), std::move(cloned_new_type));
}

Assignment::Assignment(Token token, std::unique_ptr<DeclHolder> lhs, std::unique_ptr<Node> rhs)
    : Node(NK_Assignment, token), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) { }

Assignment *Assignment::clone() const {
    auto cloned_lhs = unique_ptr<DeclHolder>(m_lhs->clone());

    auto cloned_rhs = unique_ptr<Node>(m_rhs->clone());

    return new Assignment(token(), std::move(cloned_lhs), std::move(cloned_rhs));
}

Selector::Selector(Token token, std::unique_ptr<Node> operand, std::unique_ptr<ParamName> field)
    : Node(NK_Selector, token), m_operand(std::move(operand)), m_field(std::move(field)) { }

Selector::Selector(Token token, std::unique_ptr<Node> operand, std::string field)
    : Selector(token, std::move(operand), std::make_unique<ParamName>(token, field)) { }

Selector *Selector::clone() const {
    auto cloned_operand = unique_ptr<Node>(m_operand->clone());

    auto cloned_field = unique_ptr<ParamName>(m_field->clone());

    return new Selector(token(), std::move(cloned_operand), std::move(cloned_field));
}

While::While(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> body)
    : Node(NK_While, token), m_condition(std::move(condition)), m_body(std::move(body)) { }

While *While::clone() const {
    auto cloned_condition = unique_ptr<Node>(m_condition->clone());

    auto cloned_body = unique_ptr<Node>(m_body->clone());

    return new While(token(), std::move(cloned_condition), std::move(cloned_body));
}

If::If(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> true_case, std::unique_ptr<Node> false_case)
    : Node(NK_If, token), m_condition(std::move(condition)), m_true_case(std::move(true_case)), m_false_case(std::move(false_case)) { }

If *If::clone() const {
    auto cloned_condition = unique_ptr<Node>(m_condition->clone());

    auto cloned_true_case = unique_ptr<Node>(m_true_case->clone());

    unique_ptr<Node> cloned_false_case;
    if (m_false_case) {
        cloned_false_case = unique_ptr<Node>(m_false_case->clone());
    }

    return new If(token(), std::move(cloned_condition), std::move(cloned_true_case), std::move(cloned_false_case));
}

Return::Return(Token token, std::unique_ptr<Node> expression)
    : Node(NK_Return, token), m_expression(std::move(expression)) { }

Return *Return::clone() const {
    auto cloned_expression = unique_ptr<Node>(m_expression->clone());

    return new Return(token(), std::move(cloned_expression));
}

Spawn::Spawn(Token token, std::unique_ptr<Call> call)
    : Node(NK_Spawn, token), m_call(std::move(call)) { }

Spawn *Spawn::clone() const {
    auto cloned_call = unique_ptr<Call>(m_call->clone());

    return new Spawn(token(), std::move(cloned_call));
}

Case::Case(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> assignment, std::unique_ptr<Node> body)
    : Node(NK_Case, token), m_condition(std::move(condition)), m_assignment(std::move(assignment)), m_body(std::move(body)) { }

Case *Case::clone() const {
    auto cloned_condition = unique_ptr<Node>(m_condition->clone());

    auto cloned_assignment = unique_ptr<Node>(m_assignment->clone());

    auto cloned_body = unique_ptr<Node>(m_body->clone());

    return new Case(token(), std::move(cloned_condition), std::move(cloned_assignment), std::move(cloned_body));
}

Switch::Switch(Token token, std::unique_ptr<Node> expression, std::vector<std::unique_ptr<Case>> cases, std::unique_ptr<Node> default_case)
    : Node(NK_Switch, token), m_expression(std::move(expression)), m_cases(std::move(cases)), m_default_case(std::move(default_case)) { }

Switch *Switch::clone() const {
    auto cloned_expression = unique_ptr<Node>(m_expression->clone());

    std::vector<unique_ptr<Case>> cloned_cases;
    for (auto &case_ : m_cases) {
        cloned_cases.push_back(unique_ptr<Case>(case_->clone()));
    }

    auto cloned_default_case = unique_ptr<Node>(m_default_case->clone());

    return new Switch(token(), std::move(cloned_expression), std::move(cloned_cases), std::move(cloned_default_case));
}

Let::Let(Token token, std::unique_ptr<Assignment> assignment)
    : Node(NK_Let, token), m_assignment(std::move(assignment)) { }

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

Let *Let::clone() const {
    auto cloned_assignment = unique_ptr<Assignment>(m_assignment->clone());

    return new Let(token(), std::move(cloned_assignment));
}

Parameter::Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<TypeName> given_type)
    : Node(NK_Parameter, token), m_inout(inout), m_name(std::move(name)), m_given_type(std::move(given_type)) { }

Parameter *Parameter::clone() const {
    auto cloned_name = unique_ptr<Name>(m_name->clone());

    auto cloned_given_type = unique_ptr<TypeName>(m_given_type->clone());

    return new Parameter(token(), m_inout, std::move(cloned_name), std::move(cloned_given_type));
}

DefDecl::DefDecl(Token token, std::unique_ptr<DeclName> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Node> body, std::unique_ptr<TypeName> return_type)
    : DeclNode(NK_DefDecl, token, builtin, std::move(name)), m_parameters(std::move(parameters)), m_return_type(std::move(return_type)), m_body(std::move(body)) { }

DefDecl *DefDecl::clone() const {
    auto cloned_name = unique_ptr<DeclName>(m_name->clone());

    std::vector<unique_ptr<Parameter>> cloned_parameters;
    for (auto &parameter : m_parameters) {
        cloned_parameters.push_back(unique_ptr<Parameter>(parameter->clone()));
    }

    unique_ptr<Node> cloned_body;
    if (!m_builtin) {
        cloned_body = unique_ptr<Node>(m_body->clone());
    }

    auto cloned_return_type = unique_ptr<TypeName>(m_return_type->clone());

    return new DefDecl(token(), std::move(cloned_name), m_builtin, std::move(cloned_parameters), std::move(cloned_body), std::move(cloned_return_type));
}

TypeDecl::TypeDecl(Token token, std::unique_ptr<DeclName> name)
    : DeclNode(NK_TypeDecl, token, true, std::move(name)) { }

TypeDecl::TypeDecl(Token token, std::unique_ptr<DeclName> name, std::unique_ptr<TypeName> alias)
    : DeclNode(NK_TypeDecl, token, false, std::move(name)), m_alias(std::move(alias)) { }

TypeDecl::TypeDecl(Token token, std::unique_ptr<DeclName> name, std::vector<std::unique_ptr<Name>> field_names, std::vector<std::unique_ptr<TypeName>> field_types)
    : DeclNode(NK_TypeDecl, token, false, std::move(name)), m_field_names(std::move(field_names)), m_field_types(std::move(field_types)) { }

TypeDecl *TypeDecl::clone() const {
    auto cloned_name = unique_ptr<DeclName>(m_name->clone());

    if (m_builtin) {
        return new TypeDecl(token(), std::move(cloned_name));
    } else if (m_alias) {
        auto cloned_alias = unique_ptr<TypeName>(m_alias->clone());

        return new TypeDecl(token(), std::move(cloned_name), std::move(cloned_alias));
    } else {
        std::vector<unique_ptr<Name>> cloned_field_names;
        for (auto &field_name : m_field_names) {
            cloned_field_names.push_back(unique_ptr<Name>(field_name->clone()));
        }

        std::vector<unique_ptr<TypeName>> cloned_field_types;
        for (auto &field_type : m_field_types) {
            cloned_field_types.push_back(unique_ptr<TypeName>(field_type->clone()));
        }

        return new TypeDecl(token(), std::move(cloned_name), std::move(cloned_field_names), std::move(cloned_field_types));
    }
}

ModuleDecl::ModuleDecl(Token token, std::unique_ptr<DeclName> name, std::unique_ptr<Block> body)
    : DeclNode(NK_ModuleDecl, token, false, std::move(name)), m_body(std::move(body)) { }

ModuleDecl *ModuleDecl::clone() const {
    auto cloned_name = unique_ptr<DeclName>(m_name->clone());

    auto cloned_body = unique_ptr<Block>(m_body->clone());

    return new ModuleDecl(token(), std::move(cloned_name), std::move(cloned_body));
}

Import::Import(Token token, std::unique_ptr<String> path)
    : Node(NK_Import, token), m_path(std::move(path)) { }

Import *Import::clone() const {
    auto cloned_path = unique_ptr<String>(m_path->clone());

    return new Import(token(), std::move(cloned_path));
}

SourceFile::SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code)
    : Node(NK_SourceFile, token), m_name(name), m_imports(std::move(imports)), m_code(std::move(code)) { }

SourceFile *SourceFile::clone() const {
    std::vector<unique_ptr<SourceFile>> cloned_imports;
    for (auto &import : m_imports) {
        cloned_imports.push_back(unique_ptr<SourceFile>(import->clone()));
    }

    auto cloned_code = unique_ptr<Block>(m_code->clone());

    return new SourceFile(token(), m_name, std::move(cloned_imports), std::move(cloned_code));
}
