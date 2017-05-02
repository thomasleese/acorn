//
// Created by Thomas Leese on 12/01/2017.
//

#ifndef ACORN_AST_H
#define ACORN_AST_H

#include <map>
#include <memory>
#include <string>
#include <vector>

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
            explicit Node(Token token);
            virtual ~Node();

            virtual void accept(Visitor *visitor) = 0;

            Token token() const;

        private:
            Token m_token;
        };

        class Expression : public Node {
        public:
            explicit Expression(Token token);

            types::Type *type() const;
            virtual void set_type(types::Type *type);
            bool has_type() const;
            void copy_type_from(Expression *expression);
            bool has_compatible_type_with(Expression *expression) const;
            std::string type_name() const;

        private:
            types::Type *m_type;
        };

        class Block : public Expression {
        public:
            using Expression::Expression;

            std::vector<Expression *> expressions() const;
            void add_expression(Expression *expression);
            void insert_expression(int index, Expression *expression);
            bool empty() const;

            void accept(Visitor *visitor);

        private:
            std::vector<Expression *> m_expressions;
        };

        class Name : public Expression {
        public:
            Name(Token token);
            Name(Token token, std::string name);

            bool has_parameters() const;
            std::string collapsed_value() const;
            void collapse_parameters();

            std::string value() const;

            std::vector<Name *> parameters() const;
            void add_parameter(Name *identifier);

            void accept(Visitor *visitor);

        private:
            std::string m_value;
            std::vector<Name *> m_parameters;
        };

        class VariableDeclaration : public Expression {
        public:
            VariableDeclaration(Token token, Name *name, Name *type = nullptr, bool builtin = false);

            Name *name() const;

            bool has_given_type();
            Name *given_type() const;

            bool builtin() const;

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

            std::string value() const;

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class Float : public Expression {
        public:
            Float(Token token, std::string value);

            std::string value() const;

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class Complex : public Expression {
        public:
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        class String : public Expression {
        public:
            String(Token token, std::string value);

            std::string value() const;

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class List : public Expression {
        public:
            List(Token token, std::vector<Expression *> elements);

            std::vector<Expression *> elements() const;
            Expression *get_element(size_t index) const;
            size_t no_elements() const;

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression> > m_elements;
        };

        class Dictionary : public Expression {
        public:
            Dictionary(Token token, std::vector<Expression *> keys, std::vector<Expression *> values);

            std::vector<Expression *> keys() const;
            Expression *get_key(size_t index) const;

            std::vector<Expression *> values() const;
            Expression *get_value(size_t index) const;

            size_t no_elements() const;

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression> > m_keys;
            std::vector<std::unique_ptr<Expression> > m_values;
        };

        class Tuple : public Expression {
        public:
            Tuple(Token token, std::vector<Expression *> elements);

            std::vector<Expression *> elements();

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression> > m_elements;
        };

        class Call : public Expression {
        public:
            Call(Token token, Expression *operand = nullptr);
            Call(Token token, std::string name, Expression *arg1 = nullptr, Expression *arg2 = nullptr);

            Expression *operand;
            std::map<types::ParameterType *, types::Type *> inferred_type_parameters;

            std::vector<Expression *> positional_arguments() const;
            std::vector<types::Type *> positional_argument_types() const;
            void add_positional_argument(Expression *argument);

            std::map<std::string, Expression *> keyword_arguments() const;
            std::map<std::string, types::Type *> keyword_argument_types() const;
            void add_keyword_argument(std::string name, Expression *argument);

            void set_method_index(int index);
            int get_method_index() const;

            void set_method_specialisation_index(int index);
            int get_method_specialisation_index() const;

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression> > m_positional_arguments;
            std::map<std::string, std::unique_ptr<Expression> > m_keyword_arguments;

            int m_method_index;
            int m_method_specialisation_index;
        };

        class CCall : public Expression {
        public:
            CCall(Token token, Name *name, std::vector<Name *> parameters, Name *given_return_type, std::vector<Expression *> arguments);

            Name *name() const;
            std::vector<Name *> parameters() const;
            Name *given_return_type() const;
            std::vector<Expression *> arguments() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Name> m_name;
            std::vector<std::unique_ptr<Name> > m_parameters;
            std::unique_ptr<Name> m_given_return_type;
            std::vector<std::unique_ptr<Expression> > m_arguments;
        };

        class Cast : public Expression {
        public:
            explicit Cast(Token token);
            Cast(Token token, Expression *operand, Name *new_type);

            Expression *operand;
            Name *new_type;

            void accept(Visitor *visitor);
        };

        class Assignment : public Expression {
        public:
            Assignment(Token token, VariableDeclaration *lhs, Expression *rhs);

            VariableDeclaration *lhs;
            Expression *rhs;

            bool builtin() const;

            void accept(Visitor *visitor);
        };

        class Selector : public Expression {
        public:
            Selector(Token token, Expression *operand, Name *field);
            Selector(Token token, Expression *operand, std::string field);

            Expression *operand;
            Name *name;

            void accept(Visitor *visitor);
        };

        class While : public Expression {
        public:
            While(Token token, Expression *condition, Expression *body);

            Expression *condition() const;
            Expression *body() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Expression> m_condition;
            std::unique_ptr<Expression> m_body;
        };

        class If : public Expression {
        public:
            using Expression::Expression;

            Expression *condition;
            Expression *true_case;
            Expression *false_case;

            void accept(Visitor *visitor);
        };

        class Return : public Expression {
        public:
            explicit Return(Token token);
            Return(Token token, Expression *expression);

            Expression *expression;

            void accept(Visitor *visitor);
        };

        class Spawn : public Expression {
        public:
            Spawn(Token token, Call *call);

            Call *call;

            void accept(Visitor *visitor);
        };

        class Case : public Expression {
        public:
            Case(Token token, Expression *condition, Expression *assignment, Expression *body);

            Expression *condition() const;
            Expression *assignment() const;
            Expression *body() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Expression> m_condition;
            std::unique_ptr<Expression> m_assignment;
            std::unique_ptr<Expression> m_body;
        };

        class Switch : public Expression {
        public:
            Switch(Token token, Expression *expression, std::vector<Case *> cases, Expression *default_case = nullptr);

            Expression *expression() const;
            std::vector<Case *> cases() const;
            Expression *default_case() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Expression> m_expression;
            std::vector<std::unique_ptr<Case> > m_cases;
            std::unique_ptr<Expression> m_default_case;
        };

        class Let : public Expression {
        public:
            explicit Let(Token token);
            Let(Token token, std::string name, Expression *value = nullptr, Expression *body = nullptr);

            Assignment *assignment;

            Expression *body() const;
            bool has_body() const;
            void set_body(Expression *body);

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Expression> m_body;
        };

        class Parameter : public Expression {
        public:
            explicit Parameter(Token token, bool inout, Name *name, Name *given_type);

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
            Def(Token token, Expression *name, bool builtin, std::vector<Parameter *> parameters, Expression *body, Name *given_return_type = nullptr);

            Expression *name() const;

            bool builtin() const;

            std::vector<Parameter *> parameters() const;
            Parameter *get_parameter(size_t index) const;
            size_t no_parameters() const;

            Expression *body() const;

            Name *given_return_type() const;
            bool has_given_return_type() const;

            void set_type(types::Type *type) override;

            void accept(Visitor *visitor) override;

        private:
            std::unique_ptr<Expression> m_name;
            bool m_builtin;
            std::vector<std::unique_ptr<Parameter> > m_parameters;
            std::unique_ptr<Expression> m_body;
            std::unique_ptr<Name> m_given_return_type;
        };

        class Type : public Expression {
        public:
            Type(Token token, Name *name, bool builtin);

            Name *name() const;
            void set_name(Name *name);

            bool builtin() const;
            void set_builtin(bool builtin);

            void set_type(types::Type *type) override;

            Name *alias;

            std::vector<Name *> field_names;
            std::vector<Name *> field_types;

            void accept(Visitor *visitor) override;

        private:
            std::unique_ptr<Name> m_name;
            bool m_builtin;
        };

        class MethodSignature : public Node {
        public:
            MethodSignature(Token token, Name *name, std::vector<Name *> parameter_types, Name *return_type);

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
            Module(Token token, Name *name, Block *body);

            Name *name() const;
            Block *body() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Name> m_name;
            std::unique_ptr<Block> m_body;
        };

        class Import : public Expression {
        public:
            Import(Token token, String *path);

            String *path;

            void accept(Visitor *visitor);
        };

        class SourceFile : public Expression {
        public:
            SourceFile(Token token, std::string name);

            std::string name;
            Block *code;
            std::vector<Import *> imports;

            void accept(Visitor *visitor);
        };

        class Visitor {
        public:
            virtual ~Visitor();

            virtual void visit(Block *block) = 0;
            virtual void visit(Name *identifier) = 0;
            virtual void visit(VariableDeclaration *node) = 0;
            virtual void visit(Int *expression) = 0;
            virtual void visit(Float *expression) = 0;
            virtual void visit(Complex *expression) = 0;
            virtual void visit(String *expression) = 0;
            virtual void visit(List *expression) = 0;
            virtual void visit(Dictionary *expression) = 0;
            virtual void visit(Tuple *expression) = 0;
            virtual void visit(Call *expression) = 0;
            virtual void visit(CCall *expression) = 0;
            virtual void visit(Cast *expression) = 0;
            virtual void visit(Assignment *expression) = 0;
            virtual void visit(Selector *expression) = 0;
            virtual void visit(While *expression) = 0;
            virtual void visit(If *expression) = 0;
            virtual void visit(Return *expression) = 0;
            virtual void visit(Spawn *expression) = 0;
            virtual void visit(Switch *expression) = 0;
            virtual void visit(Parameter *parameter) = 0;
            virtual void visit(Let *definition) = 0;
            virtual void visit(Def *definition) = 0;
            virtual void visit(Type *definition) = 0;
            virtual void visit(Module *module) = 0;
            virtual void visit(Import *Expression) = 0;
            virtual void visit(SourceFile *module) = 0;
        };

    }

}

#endif // ACORN_AST_H
