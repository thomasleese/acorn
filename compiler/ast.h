//
// Created by Thomas Leese on 12/01/2017.
//

#ifndef ACORN_AST_H
#define ACORN_AST_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/Support/Casting.h>

#include "token.h"

namespace acorn {

    namespace types {
        class Type;
        class ParameterType;
    }

    namespace ast {

        class Visitor;

        class Node {
        public:
            enum NodeKind {
                NK_Name,
                NK_Call,
                NK_Other
            };

            Node(NodeKind kind, Token token);
            explicit Node(Token token);
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
            explicit Expression(Token token);

            types::Type *type() const { return m_type; }
            virtual void set_type(types::Type *type) { m_type = type; }
            bool has_type() const { return m_type != nullptr; }
            void copy_type_from(Expression *expression);
            bool has_compatible_type_with(Expression *expression) const;
            std::string type_name() const;

        private:
            types::Type *m_type;
        };

        class Block : public Expression {
        public:
            explicit Block(Token token);
            Block(Token token, std::vector<std::unique_ptr<Expression>> expressions);

            std::vector<Expression *> expressions() const;
            Expression *last_expression() const { return m_expressions.back().get(); }
            bool empty() const { return m_expressions.empty(); }

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression>> m_expressions;
        };

        class Name : public Expression {
        public:
            Name(Token token, std::string value);
            Name(Token token, std::string value, std::vector<std::unique_ptr<Name>> parameters);

            std::string value() const { return m_value; }

            bool has_parameters() const { return !m_parameters.empty(); }
            std::vector<Name *> parameters() const;

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

            void set_type(types::Type *type) override;

            void accept(Visitor *visitor) override;

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

        private:
            std::string m_value;
        };

        class Float : public Expression {
        public:
            Float(Token token, std::string value);

            std::string value() const { return m_value; }

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class Complex : public Expression {
        public:
            using Expression::Expression;

            void accept(Visitor *visitor);
        };

        class String : public Expression {
        public:
            String(Token token, std::string value);

            std::string value() const { return m_value; }

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class List : public Expression {
        public:
            List(Token token, std::vector<std::unique_ptr<Expression>> elements);

            bool has_elements() const { return !m_elements.empty(); }
            size_t elements_size() const { return m_elements.size(); }
            Expression *element(size_t index) const { return m_elements[index].get(); }
            std::vector<Expression *> elements() const;

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression>> m_elements;
        };

        class Dictionary : public Expression {
        public:
            Dictionary(Token token, std::vector<std::unique_ptr<Expression>> keys, std::vector<std::unique_ptr<Expression>> values);

            bool has_elements() const { return !m_keys.empty(); }
            size_t no_elements() const { return m_keys.size(); }
            Expression *key(size_t index) const { return m_keys[index].get(); }
            Expression *value(size_t index) const { return m_values[index].get(); }

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression>> m_keys;
            std::vector<std::unique_ptr<Expression>> m_values;
        };

        class Tuple : public Expression {
        public:
            Tuple(Token token, std::vector<std::unique_ptr<Expression>> elements);

            bool has_elements() const { return !m_elements.empty(); }
            size_t elements_size() const { return m_elements.size(); }
            Expression *element(size_t index) const { return m_elements[index].get(); }
            std::vector<Expression *> elements() const;

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression>> m_elements;
        };

        class Call : public Expression {
        public:
            Call(Token token, std::unique_ptr<Expression> operand, std::vector<std::unique_ptr<Expression>> positional_arguments, std::map<std::string, std::unique_ptr<Expression>> keyword_arguments);
            Call(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Expression> arg1 = nullptr, std::unique_ptr<Expression> arg2 = nullptr);
            Call(Token token, std::string name, std::unique_ptr<Expression> arg1 = nullptr, std::unique_ptr<Expression> arg2 = nullptr);
            Call(Token token, std::string name, std::vector<std::unique_ptr<Expression>> arguments);

            Expression *operand() const { return m_operand.get(); }
            types::Type *operand_type() const { return m_operand->type(); }

            std::map<types::ParameterType *, types::Type *> inferred_type_parameters;

            std::vector<Expression *> positional_arguments() const;
            std::vector<types::Type *> positional_argument_types() const;

            std::map<std::string, Expression *> keyword_arguments() const;
            std::map<std::string, types::Type *> keyword_argument_types() const;

            void set_method_index(int index);
            int get_method_index() const;

            void set_method_specialisation_index(int index);
            int get_method_specialisation_index() const;

            void accept(Visitor *visitor);

            static bool classof(const Node *node) {
                return node->kind() == NK_Call;
            }

        private:
            std::unique_ptr<Expression> m_operand;

            std::vector<std::unique_ptr<Expression>> m_positional_arguments;
            std::map<std::string, std::unique_ptr<Expression>> m_keyword_arguments;

            int m_method_index;
            int m_method_specialisation_index;
        };

        class CCall : public Expression {
        public:
            CCall(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameters, std::unique_ptr<Name> given_return_type, std::vector<std::unique_ptr<Expression>> arguments);

            Name *name() const;
            std::vector<Name *> parameters() const;
            Name *given_return_type() const;

            size_t no_arguments() const { return m_arguments.size(); }
            Expression &argument(size_t index) const { return *m_arguments[index]; }

            void accept(Visitor *visitor);

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

        private:
            std::unique_ptr<VariableDeclaration> m_lhs;
            std::unique_ptr<Expression> m_rhs;
        };

        class Selector : public Expression {
        public:
            Selector(Token token, std::unique_ptr<Expression> operand, std::unique_ptr<Name> field);
            Selector(Token token, std::unique_ptr<Expression> operand, std::string field);

            Expression *operand() const { return m_operand.get(); }
            Name *field() const { return m_field.get(); }

            void accept(Visitor *visitor);

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

        private:
            std::unique_ptr<Expression> m_expression;
        };

        class Spawn : public Expression {
        public:
            Spawn(Token token, std::unique_ptr<Call> call);

            Call *call() const { return m_call.get(); }

            void accept(Visitor *visitor);

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

        private:
            std::unique_ptr<Expression> m_condition;
            std::unique_ptr<Expression> m_assignment;
            std::unique_ptr<Expression> m_body;
        };

        class Switch : public Expression {
        public:
            Switch(Token token, std::unique_ptr<Expression> expression, std::vector<std::unique_ptr<Case>> cases, std::unique_ptr<Expression> default_case = nullptr);

            Expression *expression() const { return m_expression.get(); }

            std::vector<Case *> cases() const;

            bool has_default_case() const { return static_cast<bool>(m_default_case); }
            Expression *default_case() const { return m_default_case.get(); }

            void accept(Visitor *visitor);

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

        private:
            std::unique_ptr<Assignment> m_assignment;
            std::unique_ptr<Expression> m_body;
        };

        class Parameter : public Expression {
        public:
            explicit Parameter(Token token, bool inout, std::unique_ptr<Name> name, std::unique_ptr<Name> given_type);

            bool inout() const;
            Name *name() const;
            Name *given_type() const;

            void accept(Visitor *visitor);

        private:
            bool m_inout;
            std::unique_ptr<Name> m_name;
            std::unique_ptr<Name> m_given_type;
        };

        class Def : public Expression {
        public:
            Def(Token token, std::unique_ptr<Expression> name, bool builtin, std::vector<std::unique_ptr<Parameter>> parameters, std::unique_ptr<Expression> body, std::unique_ptr<Name> given_return_type = nullptr);

            Expression *name() const { return m_name.get(); }

            bool builtin() const;

            std::vector<Parameter *> parameters() const;
            Parameter *parameter(size_t index) const;
            size_t no_parameters() const;

            Expression *body() const { return m_body.get(); }

            Name *given_return_type() const;
            bool has_given_return_type() const;

            void set_type(types::Type *type) override;

            void accept(Visitor *visitor) override;

        private:
            std::unique_ptr<Expression> m_name;
            bool m_builtin;
            std::vector<std::unique_ptr<Parameter>> m_parameters;
            std::unique_ptr<Expression> m_body;
            std::unique_ptr<Name> m_given_return_type;
        };

        class Type : public Expression {
        public:
            Type(Token token, std::unique_ptr<Name> name, bool builtin);

            Name *name() const;
            void set_name(Name *name);

            bool builtin() const;
            void set_builtin(bool builtin);

            void set_type(types::Type *type) override;

            bool has_alias() const { return static_cast<bool>(m_alias); }
            Name *alias() const { return m_alias.get(); }
            void set_alias(std::unique_ptr<Name> alias) { m_alias = std::move(alias); }

            void add_field(std::unique_ptr<Name> name, std::unique_ptr<Name> type);
            std::vector<Name *> field_names() const;
            std::vector<Name *> field_types() const;

            void accept(Visitor *visitor) override;

        private:
            std::unique_ptr<Name> m_name;
            std::unique_ptr<Name> m_alias;
            std::vector<std::unique_ptr<Name>> m_field_names;
            std::vector<std::unique_ptr<Name>> m_field_types;
            bool m_builtin;
        };

        class MethodSignature : public Node {
        public:
            MethodSignature(Token token, std::unique_ptr<Name> name, std::vector<std::unique_ptr<Name>> parameter_types, std::unique_ptr<Name> return_type);

            Name *name() const;
            std::vector<Name *> parameter_types() const;
            Name *return_type() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Name> m_name;
            std::vector<std::unique_ptr<Name>> m_parameter_types;
            std::unique_ptr<Name> m_return_type;
        };

        class Module : public Expression {
        public:
            Module(Token token, std::unique_ptr<Name> name, std::unique_ptr<Block> body);

            Name *name() const { return m_name.get(); }
            Block *body() const { return m_body.get(); }

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Name> m_name;
            std::unique_ptr<Block> m_body;
        };

        class Import : public Expression {
        public:
            Import(Token token, std::unique_ptr<String> path);

            String *path() const { return m_path.get(); }

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<String> m_path;
        };

        class SourceFile : public Expression {
        public:
            SourceFile(Token token, std::string name, std::vector<std::unique_ptr<SourceFile>> imports, std::unique_ptr<Block> code);

            std::string name() const { return m_name; }

            size_t no_imports() const { return m_imports.size(); }
            SourceFile *import(size_t index) const { return m_imports[index].get(); }
            std::vector<SourceFile *> imports() const;

            Block *code() const { return m_code.get(); }

            void accept(Visitor *visitor);

        private:
            std::string m_name;
            std::vector<std::unique_ptr<SourceFile>> m_imports;
            std::unique_ptr<Block> m_code;
        };

        class Visitor {
        public:
            virtual ~Visitor();

            virtual void visit(Block *node) = 0;
            virtual void visit(Name *node) = 0;
            virtual void visit(VariableDeclaration *node) = 0;
            virtual void visit(Int *node) = 0;
            virtual void visit(Float *node) = 0;
            virtual void visit(Complex *node) = 0;
            virtual void visit(String *node) = 0;
            virtual void visit(List *node) = 0;
            virtual void visit(Dictionary *node) = 0;
            virtual void visit(Tuple *node) = 0;
            virtual void visit(Call *node) = 0;
            virtual void visit(CCall *node) = 0;
            virtual void visit(Cast *node) = 0;
            virtual void visit(Assignment *node) = 0;
            virtual void visit(Selector *node) = 0;
            virtual void visit(While *node) = 0;
            virtual void visit(If *node) = 0;
            virtual void visit(Return *node) = 0;
            virtual void visit(Spawn *node) = 0;
            virtual void visit(Switch *node) = 0;
            virtual void visit(Parameter *node) = 0;
            virtual void visit(Let *node) = 0;
            virtual void visit(Def *node) = 0;
            virtual void visit(Type *node) = 0;
            virtual void visit(Module *node) = 0;
            virtual void visit(Import *node) = 0;
            virtual void visit(SourceFile *node) = 0;
        };

    }

}

#endif // ACORN_AST_H
