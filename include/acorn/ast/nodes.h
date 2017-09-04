//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/Support/Casting.h>

#include "../parser/token.h"

namespace acorn {

    namespace typesystem {
        class Type;
        class ParameterType;
    }

    using parser::Token;
}

namespace acorn::ast {

    class Visitor;

    class Node {
    public:
        enum NodeKind {
            NK_Block,
            NK_Name,
            NK_Selector,
            NK_TypeName,
            NK_DeclName,
            NK_VariableDeclaration,
            NK_Int,
            NK_Float,
            NK_Complex,
            NK_String,
            NK_List,
            NK_Tuple,
            NK_Dictionary,
            NK_Call,
            NK_CCall,
            NK_Cast,
            NK_Assignment,
            NK_While,
            NK_If,
            NK_Return,
            NK_Spawn,
            NK_Case,
            NK_Switch,
            NK_Let,
            NK_Parameter,
            NK_DefInstance,
            NK_Def,
            NK_TypeDecl,
            NK_Module,
            NK_Import,
            NK_SourceFile,
        };

        Node(NodeKind kind, Token token);
        virtual ~Node() = default;

        std::string to_string() const;

        NodeKind kind() const { return m_kind; }
        std::string kind_string() const;

        Token token() const { return m_token; }

        typesystem::Type *type() const { return m_type; }

        virtual void set_type(typesystem::Type *type) { m_type = type; }

        bool has_type() const { return m_type != nullptr; }

        void copy_type_from(Node *node);

        void copy_type_from(std::unique_ptr<Node> &node) {
            copy_type_from(node.get());
        }

        bool has_compatible_type_with(Node *node) const;

        std::string type_name() const;

    private:
        const NodeKind m_kind;
        const Token m_token;
        typesystem::Type *m_type;
    };

    class Block : public Node {
    public:
        Block(Token token, std::vector<std::unique_ptr<Node>> expressions);

        std::vector<std::unique_ptr<Node>> &expressions() {
            return m_expressions;
        }

        static bool classof(const Node *node) {
            return node->kind() == NK_Block;
        }

    private:
        std::vector<std::unique_ptr<Node>> m_expressions;
    };

    class Name : public Node {
    public:
        Name(Token token, std::string value);
        Name(Token token, std::string value, std::vector<std::unique_ptr<Name>> parameters);

        const std::string &value() const {
            return m_value;
        }

        std::vector<std::unique_ptr<Name>> &parameters() {
            return m_parameters;
        }

        bool has_parameters() const {
            return m_parameters.size() > 0;
        }

        static bool classof(const Node *node) {
            return node->kind() == NK_Name;
        }

    private:
        std::string m_value;
        std::vector<std::unique_ptr<Name>> m_parameters;
    };

    class Selector : public Node {
    public:
        Selector(Token token, std::unique_ptr<Node> operand, std::unique_ptr<Name> field);
        Selector(Token token, std::unique_ptr<Node> operand, std::string field);

        std::unique_ptr<Node> &operand() { return m_operand; }

        std::unique_ptr<Name> &field() { return m_field; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Selector;
        }

    private:
        std::unique_ptr<Node> m_operand;
        std::unique_ptr<Name> m_field;
    };

    class TypeName : public Node {
    public:
        TypeName(Token token, std::string value, std::vector<std::unique_ptr<TypeName>> parameters);

        const std::string &value() const {
            return m_value;
        }

        std::vector<std::unique_ptr<TypeName>> &parameters() {
            return m_parameters;
        }

        bool has_parameters() const {
            return m_parameters.size() > 0;
        }

        static bool classof(const Node *node) {
            return node->kind() == NK_TypeName;
        }

    private:
        std::string m_value;
        std::vector<std::unique_ptr<TypeName>> m_parameters;
    };

    class DeclName : public Node {
    public:
        DeclName(Token token, std::unique_ptr<Selector> selector, std::vector<std::unique_ptr<Name>> parameters);

        std::unique_ptr<Selector> &selector() {
            return m_selector;
        }

        std::vector<std::unique_ptr<Name>> &parameters() {
            return m_parameters;
        }

        bool has_parameters() const {
            return m_parameters.size() > 0;
        }

        static bool classof(const Node *node) {
            return node->kind() == NK_DeclName;
        }

    private:
        std::unique_ptr<Selector> m_selector;
        std::vector<std::unique_ptr<Name>> m_parameters;
    };

    class VariableDeclaration : public Node {
    public:
        VariableDeclaration(Token token, std::unique_ptr<Name> name, std::unique_ptr<TypeName> type = nullptr, bool builtin = false);

        std::unique_ptr<Name> &name() { return m_name; }

        std::unique_ptr<TypeName> &given_type() { return m_given_type; }

        bool builtin() const { return m_builtin; }

        void set_type(typesystem::Type *type) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_VariableDeclaration;
        }

    private:
        std::unique_ptr<Name> m_name;
        std::unique_ptr<TypeName> m_given_type;
        bool m_builtin;
    };

    class Int : public Node {
    public:
        Int(Token token, std::string value);

        std::string value() const { return m_value; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Int;
        }

    private:
        std::string m_value;
    };

    class Float : public Node {
    public:
        Float(Token token, std::string value);

        std::string value() const { return m_value; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Float;
        }

    private:
        std::string m_value;
    };

    class Complex : public Node {
    public:
        using Node::Node;

        static bool classof(const Node *node) {
            return node->kind() == NK_Complex;
        }
    };

    class String : public Node {
    public:
        String(Token token, std::string value);

        std::string value() const { return m_value; }

        static bool classof(const Node *node) {
            return node->kind() == NK_String;
        }

    private:
        std::string m_value;
    };

    class Sequence : public Node {
    public:
        Sequence(NodeKind kind, Token token, std::vector<std::unique_ptr<Node>> elements);

        std::vector<std::unique_ptr<Node>> &elements() {
            return m_elements;
        }

    private:
        std::vector<std::unique_ptr<Node>> m_elements;
    };

    class List : public Sequence {
    public:
        List(Token token, std::vector<std::unique_ptr<Node>> elements);

        static bool classof(const Node *node) {
            return node->kind() == NK_List;
        }
    };

    class Tuple : public Sequence {
    public:
        Tuple(Token token, std::vector<std::unique_ptr<Node>> elements);

        static bool classof(const Node *node) {
            return node->kind() == NK_Tuple;
        }
    };

    class Dictionary : public Node {
    public:
        Dictionary(Token token, std::vector<std::unique_ptr<Node>> keys, std::vector<std::unique_ptr<Node>> values);

        bool has_elements() const { return !m_keys.empty(); }

        std::vector<std::unique_ptr<Node>> &keys() { return m_keys; }

        std::vector<std::unique_ptr<Node>> &values() { return m_values; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Dictionary;
        }

    private:
        std::vector<std::unique_ptr<Node>> m_keys;
        std::vector<std::unique_ptr<Node>> m_values;
    };

    class Call : public Node {
    public:
        Call(Token token, std::unique_ptr<Node> operand, std::vector<std::unique_ptr<Node>> positional_arguments, std::map<std::string, std::unique_ptr<Node>> keyword_arguments);
        Call(Token token, std::unique_ptr<Node> operand, std::unique_ptr<Node> arg1 = nullptr, std::unique_ptr<Node> arg2 = nullptr);
        Call(Token token, std::string name, std::unique_ptr<Node> arg1 = nullptr, std::unique_ptr<Node> arg2 = nullptr);
        Call(Token token, std::string name, std::vector<std::unique_ptr<Node>> arguments);

        std::unique_ptr<Node> &operand() { return m_operand; }

        typesystem::Type *operand_type() const { return m_operand->type(); }

        std::vector<std::unique_ptr<Node>> &positional_arguments() {
            return m_positional_arguments;
        }

        std::vector<typesystem::Type *> positional_argument_types() const;

        std::map<std::string, std::unique_ptr<Node>> &keyword_arguments() { return m_keyword_arguments; }

        std::map<std::string, typesystem::Type *> keyword_argument_types() const;

        void add_inferred_type_parameter(typesystem::ParameterType *parameter, typesystem::Type *type) { m_inferred_type_parameters[parameter] = type; }
        std::map<typesystem::ParameterType *, typesystem::Type *> inferred_type_parameters() const { return m_inferred_type_parameters; }

        void set_method_index(int index) { m_method_index = index; }
        int get_method_index() const { return m_method_index; }

        void set_method_specialisation_index(int index) { m_method_specialisation_index = index; }
        int get_method_specialisation_index() const { return m_method_specialisation_index; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Call;
        }

    private:
        std::unique_ptr<Node> m_operand;

        std::vector<std::unique_ptr<Node>> m_positional_arguments;
        std::map<std::string, std::unique_ptr<Node>> m_keyword_arguments;

        std::map<typesystem::ParameterType *, typesystem::Type *> m_inferred_type_parameters;

        int m_method_index;
        int m_method_specialisation_index;
    };

    class CCall : public Node {
    public:
        CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<TypeName>> parameters, std::unique_ptr<TypeName> given_return_type, std::vector<std::unique_ptr<Node>> arguments);

        std::unique_ptr<Name> &name() { return m_name; }

        std::vector<std::unique_ptr<TypeName>> &parameters() { return m_parameters; }

        std::unique_ptr<TypeName> &return_type() { return m_return_type; }

        std::vector<std::unique_ptr<Node>> &arguments() { return m_arguments; }

        static bool classof(const Node *node) {
            return node->kind() == NK_CCall;
        }

    private:
        std::unique_ptr<Name> m_name;
        std::vector<std::unique_ptr<TypeName>> m_parameters;
        std::unique_ptr<TypeName> m_return_type;
        std::vector<std::unique_ptr<Node>> m_arguments;
    };

    class Cast : public Node {
    public:
        Cast(Token token, std::unique_ptr<Node> operand, std::unique_ptr<TypeName> new_type);

        std::unique_ptr<Node> &operand() { return m_operand; }

        std::unique_ptr<TypeName> &new_type() { return m_new_type; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Cast;
        }

    private:
        std::unique_ptr<Node> m_operand;
        std::unique_ptr<TypeName> m_new_type;
    };

    class Assignment : public Node {
    public:
        Assignment(Token token, std::unique_ptr<VariableDeclaration> lhs, std::unique_ptr<Node> rhs);

        std::unique_ptr<VariableDeclaration> &lhs() { return m_lhs; }

        std::unique_ptr<Node> &rhs() { return m_rhs; }

        bool builtin() const { return m_lhs->builtin(); }

        static bool classof(const Node *node) {
            return node->kind() == NK_Assignment;
        }

    private:
        std::unique_ptr<VariableDeclaration> m_lhs;
        std::unique_ptr<Node> m_rhs;
    };

    class While : public Node {
    public:
        While(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> body);

        std::unique_ptr<Node> &condition() { return m_condition; }

        std::unique_ptr<Node> &body() { return m_body; }

        static bool classof(const Node *node) {
            return node->kind() == NK_While;
        }

    private:
        std::unique_ptr<Node> m_condition;
        std::unique_ptr<Node> m_body;
    };

    class If : public Node {
    public:
        If(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> true_case, std::unique_ptr<Node> false_case);

        std::unique_ptr<Node> &condition() { return m_condition; }

        std::unique_ptr<Node> &true_case() { return m_true_case; }

        std::unique_ptr<Node> &false_case() { return m_false_case; }

        static bool classof(const Node *node) {
            return node->kind() == NK_If;
        }

    private:
        std::unique_ptr<Node> m_condition;
        std::unique_ptr<Node> m_true_case;
        std::unique_ptr<Node> m_false_case;
    };

    class Return : public Node {
    public:
        Return(Token token, std::unique_ptr<Node> expression);

        std::unique_ptr<Node> &expression() { return m_expression; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Return;
        }

    private:
        std::unique_ptr<Node> m_expression;
    };

    class Spawn : public Node {
    public:
        Spawn(Token token, std::unique_ptr<Call> call);

        std::unique_ptr<Call> &call() { return m_call; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Spawn;
        }

    private:
        std::unique_ptr<Call> m_call;
    };

    class Case : public Node {
    public:
        Case(Token token, std::unique_ptr<Node> condition, std::unique_ptr<Node> assignment, std::unique_ptr<Node> body);

        std::unique_ptr<Node> &condition() { return m_condition; }
        std::unique_ptr<Node> &assignment() { return m_assignment; }
        std::unique_ptr<Node> &body() { return m_body; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Case;
        }

    private:
        std::unique_ptr<Node> m_condition;
        std::unique_ptr<Node> m_assignment;
        std::unique_ptr<Node> m_body;
    };

    class Switch : public Node {
    public:
        Switch(Token token, std::unique_ptr<Node> expression, std::vector<std::unique_ptr<Case>> cases, std::unique_ptr<Node> default_case = nullptr);

        std::unique_ptr<Node> &expression() { return m_expression; }

        std::vector<std::unique_ptr<Case>> &cases() {
            return m_cases;
        }

        std::unique_ptr<Node> &default_case() { return m_default_case; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Switch;
        }

    private:
        std::unique_ptr<Node> m_expression;
        std::vector<std::unique_ptr<Case>> m_cases;
        std::unique_ptr<Node> m_default_case;
    };

    class Parameter : public Node {
    public:
        explicit Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<TypeName> given_type);

        bool inout() const { return m_inout; }

        std::unique_ptr<Name> &name() { return m_name; }

        std::unique_ptr<TypeName> &given_type() { return m_given_type; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Parameter;
        }

    private:
        bool m_inout;
        std::unique_ptr<Name> m_name;
        std::unique_ptr<TypeName> m_given_type;
    };

    class Let : public Node {
    public:
        Let(Token token, std::unique_ptr<Assignment> assignment, std::unique_ptr<Node> body);
        Let(Token token, std::string name, std::unique_ptr<Node> value = nullptr, std::unique_ptr<Node> body = nullptr);

        std::unique_ptr<Assignment> &assignment() { return m_assignment; }

        std::unique_ptr<Node> &body() { return m_body; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Let;
        }

    private:
        std::unique_ptr<Assignment> m_assignment;
        std::unique_ptr<Node> m_body;
    };

    class DefInstance : public Node {
    public:
        DefInstance(Token token, std::unique_ptr<Selector> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Node> body, std::unique_ptr<TypeName> return_type = nullptr);

        std::unique_ptr<Selector> &name() { return m_name; }

        bool builtin() const { return m_builtin; }

        std::vector<std::unique_ptr<Parameter>> &parameters() {
            return m_parameters;
        }

        std::unique_ptr<Node> &body() { return m_body; }

        std::unique_ptr<TypeName> &return_type() { return m_return_type; }

        void set_type(typesystem::Type *type) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_DefInstance;
        }

    private:
        bool m_builtin;
        std::unique_ptr<Selector> m_name;
        std::vector<std::unique_ptr<Parameter>> m_parameters;
        std::unique_ptr<TypeName> m_return_type;
        std::unique_ptr<Node> m_body;
    };

    class Def : public Node {
    public:
        Def(Token token, std::unique_ptr<DefInstance> main_instance);

        std::unique_ptr<DefInstance> &main_instance() {
            return m_instances[0];
        }

        std::vector<std::unique_ptr<DefInstance>> &instances() {
            return m_instances;
        }

        static bool classof(const Node *node) {
            return node->kind() == NK_Def;
        }

    private:
        std::vector<std::unique_ptr<DefInstance>> m_instances;
    };

    class TypeDecl : public Node {
    public:
        TypeDecl(Token token, std::unique_ptr<Name> name);
        TypeDecl(Token token, std::unique_ptr<Name> name, std::unique_ptr<TypeName> alias);
        TypeDecl(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> field_names, std::vector<std::unique_ptr<TypeName>> field_types);

        std::unique_ptr<Name> &name() { return m_name; }

        bool builtin() const { return m_builtin; }

        std::unique_ptr<TypeName> &alias() { return m_alias; }

        std::vector<std::unique_ptr<Name>> &field_names() {
            return m_field_names;
        }

        std::vector<std::unique_ptr<TypeName>> &field_types() {
            return m_field_types;
        }

        void set_type(typesystem::Type *type) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_TypeDecl;
        }

    private:
        std::unique_ptr<Name> m_name;
        bool m_builtin;
        std::unique_ptr<TypeName> m_alias;
        std::vector<std::unique_ptr<Name>> m_field_names;
        std::vector<std::unique_ptr<TypeName>> m_field_types;
    };

    class Module : public Node {
    public:
        Module(Token token, std::unique_ptr<Name> name, std::unique_ptr<Block> body);

        std::unique_ptr<Name> &name() { return m_name; }

        std::unique_ptr<Block> &body() { return m_body; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Module;
        }

    private:
        std::unique_ptr<Name> m_name;
        std::unique_ptr<Block> m_body;
    };

    class Import : public Node {
    public:
        Import(Token token, std::unique_ptr<String> path);

        std::unique_ptr<String> &path() { return m_path; }

        static bool classof(const Node *node) {
            return node->kind() == NK_Import;
        }

    private:
        std::unique_ptr<String> m_path;
    };

    class SourceFile : public Node {
    public:
        SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code);

        std::string name() const { return m_name; }

        std::vector<std::unique_ptr<SourceFile>> &imports() {
            return m_imports;
        }

        std::unique_ptr<Block> &code() { return m_code; }

        static bool classof(const Node *node) {
            return node->kind() == NK_SourceFile;
        }

    private:
        std::string m_name;
        std::vector<std::unique_ptr<SourceFile>> m_imports;
        std::unique_ptr<Block> m_code;
    };

}
