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

            Token token;
            types::Type *type;
        };

        class Expression : public Node {
        public:
            using Node::Node;
        };

        class Block : public Expression {
        public:
            using Expression::Expression;

            std::vector<Expression *> expressions;

            void accept(Visitor *visitor);
        };

        class Identifier : public Expression {
        public:
            Identifier(Token token);
            Identifier(Token token, std::string name);

            bool has_parameters() const;
            std::string collapsed_value() const;
            void collapse_parameters();

            std::string value;
            std::vector<Identifier *> parameters;

            void accept(Visitor *visitor);
        };

        class VariableDeclaration : public Expression {
        public:
            VariableDeclaration(Token token, Identifier *name = nullptr, Identifier *type = nullptr);

            Identifier *name() const;

            bool has_given_type();
            Identifier *given_type() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Identifier> m_name;
            std::unique_ptr<Identifier> m_given_type;
        };

        class Definition : public Node {
        public:
            using Node::Node;

            Identifier *name;
        };

        class IntegerLiteral : public Expression {
        public:
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        class FloatLiteral : public Expression {
        public:
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        class ImaginaryLiteral : public Expression {
        public:
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        class StringLiteral : public Expression {
        public:
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
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

            Identifier *name;
            std::vector<Identifier *> field_names;
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
            Call(Token token, std::string name, Expression *arg1 = nullptr, Expression *arg2 = nullptr);

            Expression *operand;
            std::vector<Expression *> arguments;
            std::map<types::ParameterType *, types::Type *> inferred_type_parameters;

            void accept(Visitor *visitor);
        };

        class CCall : public Expression {
        public:
            CCall(Token token);

            Identifier *name;
            std::vector<Identifier *> parameters;
            Identifier *returnType;
            std::vector<Expression *> arguments;

            void accept(Visitor *visitor);
        };

        class Cast : public Expression {
        public:
            Cast(Token token);

            Expression *operand;
            Identifier *new_type;

            void accept(Visitor *visitor);
        };

        class Assignment : public Expression {
        public:
            explicit Assignment(Token token, Expression *lhs, Expression *rhs);

            Expression *lhs;
            Expression *rhs;

            void accept(Visitor *visitor);
        };

        class Selector : public Expression {
        public:
            Selector(Token token, Expression *operand, Identifier *field);
            Selector(Token token, Expression *operand, std::string field);

            Expression *operand;
            Identifier *name;

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
            using Expression::Expression;

            Expression *expression;

            void accept(Visitor *visitor);
        };

        class Spawn : public Expression {
        public:
            Spawn(Token token, Call *call);

            Call *call;

            void accept(Visitor *visitor);
        };

        class Case : public Node {
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

        // misc
        class Parameter : public Node {
        public:
            explicit Parameter(Token token);

            bool inout;
            Identifier *name;
            Identifier *typeNode;

            void accept(Visitor *visitor);
        };

        // definitions
        class VariableDefinition : public Definition {
        public:
            explicit VariableDefinition(Token token);
            VariableDefinition(Token token, std::string name, Expression *value = nullptr);

            Assignment *assignment;

            void accept(Visitor *visitor);
        };

        class FunctionDefinition : public Definition {
        public:
            explicit FunctionDefinition(Token token);

            std::vector<Parameter *> parameters;
            Expression *body;
            Identifier *returnType;

            void accept(Visitor *visitor);
        };

        class TypeDefinition : public Definition {
        public:
            TypeDefinition(Token token);

            Identifier *alias;

            std::vector<Identifier *> field_names;
            std::vector<Identifier *> field_types;

            void accept(Visitor *visitor);
        };

        class MethodSignature : public Node {
        public:
            MethodSignature(Token token, Identifier *name, std::vector<Identifier *> parameter_types, Identifier *return_type);

            Identifier *name() const;
            std::vector<Identifier *> parameter_types() const;
            Identifier *return_type() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Identifier> m_name;
            std::vector<std::unique_ptr<Identifier>> m_parameter_types;
            std::unique_ptr<Identifier> m_return_type;
        };

        class DefinitionExpression : public Expression {
        public:
            explicit DefinitionExpression(Definition *definition);

            Definition *definition;

            void accept(Visitor *visitor);
        };

        class ImportExpression : public Expression {
        public:
            ImportExpression(Token token, StringLiteral *path);

            StringLiteral *path;

            void accept(Visitor *visitor);
        };

        // source file
        class SourceFile : public Node {
        public:
            SourceFile(Token token, std::string name);

            std::string name;
            Block *code;
            std::vector<ImportExpression *> imports;

            void accept(Visitor *visitor);
        };

        // Visitor
        class Visitor {
        public:
            virtual ~Visitor();

            // misc
            virtual void visit(Block *block) = 0;

            // expressions
            virtual void visit(Identifier *identifier) = 0;
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

            // misc
            virtual void visit(Parameter *parameter) = 0;

            // definitions
            virtual void visit(VariableDefinition *definition) = 0;
            virtual void visit(FunctionDefinition *definition) = 0;
            virtual void visit(TypeDefinition *definition) = 0;

            // Expressions
            virtual void visit(DefinitionExpression *Expression) = 0;
            virtual void visit(ImportExpression *Expression) = 0;

            // module
            virtual void visit(SourceFile *module) = 0;
        };

    }

}

#endif // ACORN_AST_H
