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
            VariableDeclaration(Token token, Name *name = nullptr, Name *type = nullptr);

            Name *name() const;

            bool has_given_type();
            Name *given_type() const;

            void set_type(types::Type *type) override;

            void accept(Visitor *visitor) override;

        private:
            std::unique_ptr<Name> m_name;
            std::unique_ptr<Name> m_given_type;
        };

        class Definition : public Expression {
        public:
            explicit Definition(Token token);
            Definition(Token token, Name *name);
            Definition(Token token, std::string name);

            Name *name() const;
            void set_name(Name *name);

            void set_type(types::Type *type) override;

        private:
            std::unique_ptr<Name> m_name;
        };

        class IntegerLiteral : public Expression {
        public:
            IntegerLiteral(Token token, std::string value);

            std::string value() const;

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class FloatLiteral : public Expression {
        public:
            FloatLiteral(Token token, std::string value);

            std::string value() const;

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class ImaginaryLiteral : public Expression {
        public:
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        class StringLiteral : public Expression {
        public:
            StringLiteral(Token token, std::string value);

            std::string value() const;

            void accept(Visitor *visitor);

        private:
            std::string m_value;
        };

        class SequenceLiteral : public Expression {
        public:
            using Expression::Expression;

            std::vector<Expression *> elements;

            void accept(Visitor *visitor);
        };

        class MappingLiteral : public Expression {
        public:
            using Expression::Expression;

            std::vector<Expression *> keys;
            std::vector<Expression *> values;

            void accept(Visitor *visitor);
        };

        class RecordLiteral : public Expression {
        public:
            using Expression::Expression;

            Name *name;
            std::vector<Name *> field_names;
            std::vector<Expression *> field_values;

            void accept(Visitor *visitor);
        };

        class TupleLiteral : public Expression {
        public:
            TupleLiteral(Token token, std::vector<Expression *> elements);

            std::vector<Expression *> elements();

            void accept(Visitor *visitor);

        private:
            std::vector<std::unique_ptr<Expression> > m_elements;
        };

        class Call : public Expression {
        public:
            explicit Call(Token token);
            Call(Token token, Expression *operand);
            Call(Token token, std::string name, Expression *arg1 = nullptr, Expression *arg2 = nullptr);

            Expression *operand;
            std::vector<Expression *> arguments;
            std::map<types::ParameterType *, types::Type *> inferred_type_parameters;

            void accept(Visitor *visitor);
        };

        class CCall : public Expression {
        public:
            CCall(Token token);

            Name *name;
            std::vector<Name *> parameters;
            Name *given_return_type;
            std::vector<Expression *> arguments;

            void accept(Visitor *visitor);
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
            explicit Assignment(Token token, VariableDeclaration *lhs, Expression *rhs);

            VariableDeclaration *lhs;
            Expression *rhs;

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

        class Parameter : public Expression {
        public:
            explicit Parameter(Token token);

            bool inout;
            Name *name;
            Name *typeNode;

            void accept(Visitor *visitor);
        };

        class VariableDefinition : public Expression {
        public:
            explicit VariableDefinition(Token token);
            VariableDefinition(Token token, std::string name, Expression *value = nullptr, Expression *body = nullptr);

            Assignment *assignment;

            Expression *body() const;
            void set_body(Expression *body);

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Expression> m_body;
        };

        class FunctionDefinition : public Definition {
        public:
            explicit FunctionDefinition(Token token);

            std::vector<Parameter *> parameters;
            Expression *body;
            Name *given_return_type;

            void accept(Visitor *visitor);
        };

        class TypeDefinition : public Definition {
        public:
            TypeDefinition(Token token);

            Name *alias;

            std::vector<Name *> field_names;
            std::vector<Name *> field_types;

            void accept(Visitor *visitor);
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
            Import(Token token, StringLiteral *path);

            StringLiteral *path;

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
            virtual void visit(IntegerLiteral *expression) = 0;
            virtual void visit(FloatLiteral *expression) = 0;
            virtual void visit(ImaginaryLiteral *expression) = 0;
            virtual void visit(StringLiteral *expression) = 0;
            virtual void visit(SequenceLiteral *expression) = 0;
            virtual void visit(MappingLiteral *expression) = 0;
            virtual void visit(RecordLiteral *expression) = 0;
            virtual void visit(TupleLiteral *expression) = 0;
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
            virtual void visit(VariableDefinition *definition) = 0;
            virtual void visit(FunctionDefinition *definition) = 0;
            virtual void visit(TypeDefinition *definition) = 0;
            virtual void visit(Module *module) = 0;
            virtual void visit(Import *Expression) = 0;
            virtual void visit(SourceFile *module) = 0;
        };

    }

}

#endif // ACORN_AST_H
