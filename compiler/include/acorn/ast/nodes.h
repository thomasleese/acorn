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
            NK_Selector,
            NK_While,
            NK_If,
            NK_Return,
            NK_Spawn,
            NK_Case,
            NK_Switch,
            NK_Let,
            NK_Parameter,
            NK_MethodSignature,
            NK_Def,
            NK_Type,
            NK_Module,
            NK_Import,
            NK_SourceFile,
        };

        Node(NodeKind kind, Token token);
        virtual ~Node();

        virtual void accept(Visitor *visitor) = 0;

        NodeKind kind() const { return m_kind; }
        Token token() const { return m_token; }

    private:
        const NodeKind m_kind;
        Token m_token;
    };

    class Expression : public Node {
    public:
        Expression(NodeKind kind, Token token);
        virtual ~Expression();

        typesystem::Type *type() const { return m_type; }
        virtual void set_type(typesystem::Type *type) { m_type = type; }
        bool has_type() const { return m_type != nullptr; }
        void copy_type_from(Expression *node);
        bool has_compatible_type_with(Expression *node) const;
        std::string type_name() const;

    private:
        typesystem::Type *m_type;
    };

    class Block : public Expression {
    public:
        Block(Token token, std::vector<std::unique_ptr<Expression>> expressions);

        std::vector<std::unique_ptr<Expression>> &expressions() {
            return m_expressions;
        }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Block;
        }

    private:
        std::vector<std::unique_ptr<Expression>> m_expressions;
    };

    class Name : public Expression {
    public:
        Name(Token token, std::string value);
        Name(Token token, std::string value, std::vector<std::unique_ptr<Name>> parameters);

        const std::string &value() const {
            return m_value;
        }

        const std::vector<std::unique_ptr<Name>> &parameters() const {
            return m_parameters;
        }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Name;
        }

    private:
        std::string m_value;
        std::vector<std::unique_ptr<Name>> m_parameters;
    };

    class VariableDeclaration : public Expression {
    public:
        VariableDeclaration(Token token, std::unique_ptr<Name> name, std::unique_ptr<Name> type = nullptr, bool builtin = false);

        Name *name() const { return m_name.get(); }

        bool has_given_type() { return static_cast<bool>(m_given_type); }
        Name *given_type() const { return m_given_type.get(); }

        bool builtin() const { return m_builtin; }

        void set_type(typesystem::Type *type) override;

        void accept(Visitor *visitor) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_VariableDeclaration;
        }

    private:
        std::unique_ptr<Name> m_name;
        std::unique_ptr<Name> m_given_type;
        bool m_builtin;
    };

    class Int : public Expression {
    public:
        Int(Token token, std::string value);

        std::string value() const { return m_value; }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Int;
        }

    private:
        std::string m_value;
    };

    class Float : public Expression {
    public:
        Float(Token token, std::string value);

        std::string value() const { return m_value; }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Float;
        }

    private:
        std::string m_value;
    };

    class Complex : public Expression {
    public:
        using Expression::Expression;

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Complex;
        }
    };

    class String : public Expression {
    public:
        String(Token token, std::string value);

        std::string value() const { return m_value; }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_String;
        }

    private:
        std::string m_value;
    };

    class Sequence : public Expression {
    public:
        Sequence(NodeKind kind, Token token, std::vector<std::unique_ptr<Expression>> elements);

        const std::vector<std::unique_ptr<Expression>> &elements() const {
            return m_elements;
        };

    private:
        std::vector<std::unique_ptr<Expression>> m_elements;
    };

    class List : public Sequence {
    public:
        List(Token token, std::vector<std::unique_ptr<Expression>> elements);

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_List;
        }
    };

    class Tuple : public Sequence {
    public:
        Tuple(Token token, std::vector<std::unique_ptr<Expression>> elements);

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Tuple;
        }
    };

    class Dictionary : public Expression {
    public:
        Dictionary(Token token, std::vector<std::unique_ptr<Expression>> keys, std::vector<std::unique_ptr<Expression>> values);

        bool has_elements() const { return !m_keys.empty(); }
        size_t elements_size() const { return m_keys.size(); }
        Expression *key(size_t index) const { return m_keys[index].get(); }
        Expression *value(size_t index) const { return m_values[index].get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Dictionary;
        }

    private:
        std::vector<std::unique_ptr<Expression>> m_keys;
        std::vector<std::unique_ptr<Expression>> m_values;
    };

    class Call : public Expression {
    public:
        Call(Token token, std::unique_ptr<Expression> operand, std::vector<std::unique_ptr<Expression>> positional_arguments, std::map<std::string, std::unique_ptr<Expression>> keyword_arguments);
        Call(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Expression> arg1 = nullptr, std::unique_ptr<Expression> arg2 = nullptr);
        Call(Token token, std::string name, std::unique_ptr<Expression> arg1 = nullptr, std::unique_ptr<Expression> arg2 = nullptr);
        Call(Token token, std::string name, std::vector<std::unique_ptr<Expression>> arguments);

        Expression *operand() const { return m_operand.get(); }
        typesystem::Type *operand_type() const { return m_operand->type(); }

        const std::vector<std::unique_ptr<Expression>> &positional_arguments() const {
            return m_positional_arguments;
        };

        std::vector<typesystem::Type *> positional_argument_types() const;

        std::map<std::string, Expression *> keyword_arguments() const;
        std::map<std::string, typesystem::Type *> keyword_argument_types() const;

        void add_inferred_type_parameter(typesystem::ParameterType *parameter, typesystem::Type *type) { m_inferred_type_parameters[parameter] = type; }
        std::map<typesystem::ParameterType *, typesystem::Type *> inferred_type_parameters() const { return m_inferred_type_parameters; }

        void set_method_index(int index) { m_method_index = index; }
        int get_method_index() const { return m_method_index; }

        void set_method_specialisation_index(int index) { m_method_specialisation_index = index; }
        int get_method_specialisation_index() const { return m_method_specialisation_index; }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Call;
        }

    private:
        std::unique_ptr<Expression> m_operand;

        std::vector<std::unique_ptr<Expression>> m_positional_arguments;
        std::map<std::string, std::unique_ptr<Expression>> m_keyword_arguments;

        std::map<typesystem::ParameterType *, typesystem::Type *> m_inferred_type_parameters;

        int m_method_index;
        int m_method_specialisation_index;
    };

    class CCall : public Expression {
    public:
        CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameters, std::unique_ptr<Name> given_return_type, std::vector<std::unique_ptr<Expression>> arguments);

        Name *name() const { return m_name.get(); }

        const std::vector<std::unique_ptr<Name>> &parameters() const {
            return m_parameters;
        }

        Name *given_return_type() const { return m_given_return_type.get(); }

        const std::vector<std::unique_ptr<Expression>> &arguments() const {
            return m_arguments;
        }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_CCall;
        }

    private:
        std::unique_ptr<Name> m_name;
        std::vector<std::unique_ptr<Name>> m_parameters;
        std::unique_ptr<Name> m_given_return_type;
        std::vector<std::unique_ptr<Expression>> m_arguments;
    };

    class Cast : public Expression {
    public:
        Cast(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Name> new_type);

        Expression *operand() const { return m_operand.get(); }
        Name *new_type() const { return m_new_type.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Cast;
        }

    private:
        std::unique_ptr<Expression> m_operand;
        std::unique_ptr<Name> m_new_type;
    };

    class Assignment : public Expression {
    public:
        Assignment(Token token, std::unique_ptr<VariableDeclaration> lhs, std::unique_ptr<Expression> rhs);

        VariableDeclaration *lhs() const { return m_lhs.get(); }
        Expression *rhs() const { return m_rhs.get(); }

        bool builtin() const { return lhs()->builtin(); }

        void accept(Visitor *visitor) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_Assignment;
        }

    private:
        std::unique_ptr<VariableDeclaration> m_lhs;
        std::unique_ptr<Expression> m_rhs;
    };

    class Selector : public Expression {
    public:
        Selector(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Name> field);
        Selector(Token token, std::unique_ptr<Expression> operand, std::string field);

        bool has_operand() const { return m_operand != nullptr; }
        Expression *operand() const { return m_operand.get(); }
        Name *field() const { return m_field.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Selector;
        }

    private:
        std::unique_ptr<Expression> m_operand;
        std::unique_ptr<Name> m_field;
    };

    class While : public Expression {
    public:
        While(Token token, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> body);

        Expression *condition() const { return m_condition.get(); }
        Expression *body() const { return m_condition.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_While;
        }

    private:
        std::unique_ptr<Expression> m_condition;
        std::unique_ptr<Expression> m_body;
    };

    class If : public Expression {
    public:
        If(Token token, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> true_case, std::unique_ptr<Expression> false_case);

        Expression *condition() const { return m_condition.get(); }

        Expression *true_case() const { return m_true_case.get(); }

        bool has_false_case() const { return static_cast<bool>(m_false_case); }
        Expression *false_case() const { return m_false_case.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_If;
        }

    private:
        std::unique_ptr<Expression> m_condition;
        std::unique_ptr<Expression> m_true_case;
        std::unique_ptr<Expression> m_false_case;
    };

    class Return : public Expression {
    public:
        Return(Token token, std::unique_ptr<Expression> expression);

        Expression *expression() const { return m_expression.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Return;
        }

    private:
        std::unique_ptr<Expression> m_expression;
    };

    class Spawn : public Expression {
    public:
        Spawn(Token token, std::unique_ptr<Call> call);

        Call *call() const { return m_call.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Spawn;
        }

    private:
        std::unique_ptr<Call> m_call;
    };

    class Case : public Expression {
    public:
        Case(Token token, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> assignment, std::unique_ptr<Expression> body);

        Expression *condition() const { return m_condition.get(); }

        bool has_assignment() const { return static_cast<bool>(m_assignment); }
        Expression *assignment() const { return m_assignment.get(); }

        Expression *body() const { return m_body.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Case;
        }

    private:
        std::unique_ptr<Expression> m_condition;
        std::unique_ptr<Expression> m_assignment;
        std::unique_ptr<Expression> m_body;
    };

    class Switch : public Expression {
    public:
        Switch(Token token, std::unique_ptr<Expression> expression, std::vector<std::unique_ptr<Case>> cases, std::unique_ptr<Expression> default_case = nullptr);

        Expression *expression() const { return m_expression.get(); }

        const std::vector<std::unique_ptr<Case>> &cases() const {
            return m_cases;
        }

        bool has_default_case() const { return static_cast<bool>(m_default_case); }
        Expression *default_case() const { return m_default_case.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Switch;
        }

    private:
        std::unique_ptr<Expression> m_expression;
        std::vector<std::unique_ptr<Case>> m_cases;
        std::unique_ptr<Expression> m_default_case;
    };

    class Let : public Expression {
    public:
        Let(Token token, std::unique_ptr<Assignment> assignment, std::unique_ptr<Expression> body);
        Let(Token token, std::string name, std::unique_ptr<Expression> value = nullptr, std::unique_ptr<Expression> body = nullptr);

        Assignment *assignment() const { return m_assignment.get(); }

        bool has_body() const { return static_cast<bool>(m_body); }
        Expression *body() const { return m_body.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Let;
        }

    private:
        std::unique_ptr<Assignment> m_assignment;
        std::unique_ptr<Expression> m_body;
    };

    class Parameter : public Expression {
    public:
        explicit Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<Name> given_type);

        bool inout() const { return m_inout; }

        Name *name() const { return m_name.get(); }

        bool has_given_type() const { return static_cast<bool>(m_given_type); }
        Name *given_type() const { return m_given_type.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Parameter;
        }

    private:
        bool m_inout;
        std::unique_ptr<Name> m_name;
        std::unique_ptr<Name> m_given_type;
    };

    class MethodSignature : public Node {
    public:
        MethodSignature(Token token, std::unique_ptr<Selector> name, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Name> given_return_type);

        Selector *name() const { return m_name.get(); }

        const std::vector<std::unique_ptr<Parameter>> &parameters() const {
            return m_parameters;
        }

        bool has_given_return_type() const { return static_cast<bool>(m_given_return_type); }
        Name *given_return_type() const { return m_given_return_type.get(); }

        void accept(Visitor *visitor) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_MethodSignature;
        }

    private:
        std::unique_ptr<Selector> m_name;
        std::vector<std::unique_ptr<Parameter>> m_parameters;
        std::unique_ptr<Name> m_given_return_type;
    };

    class Def : public Expression {
    public:
        Def(Token token, std::unique_ptr<Selector> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Expression> body, std::unique_ptr<Name> given_return_type = nullptr);

        Selector *name() const { return m_signature->name(); }

        bool builtin() const { return m_builtin; }

        const std::vector<std::unique_ptr<Parameter>> &parameters() const {
            return m_signature->parameters();
        }

        Expression *body() const { return m_body.get(); }

        bool has_given_return_type() const { return m_signature->has_given_return_type(); }
        Name *given_return_type() const { return m_signature->given_return_type(); }

        void set_type(typesystem::Type *type) override;

        void accept(Visitor *visitor) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_Def;
        }

    private:
        std::unique_ptr<MethodSignature> m_signature;
        bool m_builtin;
        std::unique_ptr<Expression> m_body;
    };

    class Type : public Expression {
    public:
        Type(Token token, std::unique_ptr<Name> name);
        Type(Token token, std::unique_ptr<Name> name, std::unique_ptr<Name> alias);
        Type(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> field_names, std::vector<std::unique_ptr<Name>> field_types);

        Name *name() const { return m_name.get(); }

        bool builtin() const { return m_builtin; }

        bool has_alias() const { return static_cast<bool>(m_alias); }
        Name *alias() const { return m_alias.get(); }

        const std::vector<std::unique_ptr<Name>> &field_names() const {
            return m_field_names;
        }

        const std::vector<std::unique_ptr<Name>> &field_types() const {
            return m_field_types;
        }

        void set_type(typesystem::Type *type) override;

        void accept(Visitor *visitor) override;

        static bool classof(const Node *node) {
            return node->kind() == NK_Type;
        }

    private:
        std::unique_ptr<Name> m_name;
        bool m_builtin;
        std::unique_ptr<Name> m_alias;
        std::vector<std::unique_ptr<Name>> m_field_names;
        std::vector<std::unique_ptr<Name>> m_field_types;
    };

    class Module : public Expression {
    public:
        Module(Token token, std::unique_ptr<Name> name, std::unique_ptr<Block> body);

        Name *name() const { return m_name.get(); }
        Block *body() const { return m_body.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Module;
        }

    private:
        std::unique_ptr<Name> m_name;
        std::unique_ptr<Block> m_body;
    };

    class Import : public Expression {
    public:
        Import(Token token, std::unique_ptr<String> path);

        String *path() const { return m_path.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_Import;
        }

    private:
        std::unique_ptr<String> m_path;
    };

    class SourceFile : public Expression {
    public:
        SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code);

        std::string name() const { return m_name; }

        const std::vector<std::unique_ptr<SourceFile>> &imports() const {
            return m_imports;
        }

        Block *code() const { return m_code.get(); }

        void accept(Visitor *visitor);

        static bool classof(const Node *node) {
            return node->kind() == NK_SourceFile;
        }

    private:
        std::string m_name;
        std::vector<std::unique_ptr<SourceFile>> m_imports;
        std::unique_ptr<Block> m_code;
    };

}
