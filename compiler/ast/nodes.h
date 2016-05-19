//
// Created by Thomas Leese on 07/05/2016.
//

#ifndef ACORN_AST_NODES_H
#define ACORN_AST_NODES_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace acorn {

    struct Token;

    namespace types {
        class Type;
        class ParameterConstructor;
    }

    namespace ast {

        class Visitor;

        struct Node {
            explicit Node(Token *token);
            virtual ~Node();

            virtual void accept(Visitor *visitor) = 0;

            Token *token;
            types::Type *type;
        };

        // basic categories
        struct Expression : Node {
            using Node::Node;
        };

        struct Statement : Node {
            using Node::Node;
        };

        // misc
        struct CodeBlock : Node {
            using Node::Node;

            std::vector<Statement *> statements;

            void accept(Visitor *visitor);
        };

        // expressions
        struct Identifier : Expression {
            Identifier(Token *token);
            Identifier(Token *token, std::string name);

            bool has_parameters() const;
            std::string collapsed_value() const;
            void collapse_parameters();

            std::string value;
            std::vector<Identifier *> parameters;

            void accept(Visitor *visitor);
        };

        class VariableDeclaration : public Expression {
        public:
            VariableDeclaration(Token *token, Identifier *name = nullptr, Identifier *type = nullptr);

            Identifier *name() const;

            bool has_given_type();
            Identifier *given_type() const;

            void accept(Visitor *visitor);

        private:
            std::unique_ptr<Identifier> m_name;
            std::unique_ptr<Identifier> m_given_type;
        };

        struct Definition : Node {
            using Node::Node;

            Identifier *name;
        };

        struct IntegerLiteral : Expression {
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        struct FloatLiteral : Expression {
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        struct ImaginaryLiteral : Expression {
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        struct StringLiteral : Expression {
            using Expression::Expression;

            std::string value;

            void accept(Visitor *visitor);
        };

        struct SequenceLiteral : Expression {
            using Expression::Expression;

            std::vector<Expression *> elements;

            void accept(Visitor *visitor);
        };

        struct MappingLiteral : Expression {
            using Expression::Expression;

            std::vector<Expression *> keys;
            std::vector<Expression *> values;

            void accept(Visitor *visitor);
        };

        struct RecordLiteral : Expression {
            using Expression::Expression;

            Identifier *name;
            std::vector<Identifier *> field_names;
            std::vector<Expression *> field_values;

            void accept(Visitor *visitor);
        };

        struct Call : Expression {
            Call(Token *token);
            Call(std::string name, Token *token);

            Expression *operand;
            std::vector<Expression *> arguments;
            std::map<types::ParameterConstructor *, types::Type *> inferred_type_parameters;

            void accept(Visitor *visitor);
        };

        struct CCall : Expression {
            CCall(Token *token);

            Identifier *name;
            std::vector<Identifier *> parameters;
            Identifier *returnType;
            std::vector<Expression *> arguments;

            void accept(Visitor *visitor);
        };

        struct Cast : Expression {
            Cast(Token *token);

            Expression *operand;
            Identifier *new_type;

            void accept(Visitor *visitor);
        };

        struct Assignment : Expression {
            explicit Assignment(Token *token, Expression *lhs, Expression *rhs);

            Expression *lhs;
            Expression *rhs;

            void accept(Visitor *visitor);
        };

        struct Selector : Expression {
            using Expression::Expression;

            Expression *operand;
            Identifier *name;

            void accept(Visitor *visitor);
        };

        struct Comma : Expression {
            Comma(Token *token);
            Comma(Expression *lhs, Expression *rhs, Token *token);

            Expression *lhs;
            Expression *rhs;

            void accept(Visitor *visitor);
        };

        struct While : Expression {
            using Expression::Expression;

            Expression *condition;
            CodeBlock *code;

            void accept(Visitor *visitor);
        };

        struct For : Expression {
            using Expression::Expression;

            Identifier *name;
            Expression *iterator;
            CodeBlock *code;

            void accept(Visitor *visitor);
        };

        struct If : Expression {
            using Expression::Expression;

            Expression *condition;
            CodeBlock *trueCode;
            CodeBlock *falseCode;

            void accept(Visitor *visitor);
        };

        struct Return : Expression {
            using Expression::Expression;

            Expression *expression;

            void accept(Visitor *visitor);
        };

        struct Spawn : Expression {
            Spawn(Token *token, Call *call);

            Call *call;

            void accept(Visitor *visitor);
        };

        struct Sizeof : Expression {
            Sizeof(Token *token, Identifier *identifier);

            Identifier *identifier;

            void accept(Visitor *visitor);
        };

        struct Strideof : Expression {
            Strideof(Token *token, Identifier *identifier);

            Identifier *identifier;

            void accept(Visitor *visitor);
        };

        // misc
        struct Parameter : Node {
            explicit Parameter(Token *token);

            bool inout;
            Identifier *name;
            Identifier *typeNode;

            void accept(Visitor *visitor);
        };

        // definitions
        struct VariableDefinition : Definition {
            VariableDefinition(Token *token);
            VariableDefinition(std::string name, Token *token);

            Assignment *assignment;

            void accept(Visitor *visitor);
        };

        struct FunctionDefinition : Definition {
            using Definition::Definition;

            std::vector<Parameter *> parameters;
            CodeBlock *code;
            Identifier *returnType;

            void accept(Visitor *visitor);
        };

        struct TypeDefinition : Definition {
            TypeDefinition(Token *token);

            Identifier *alias;

            std::vector<Identifier *> field_names;
            std::vector<Identifier *> field_types;

            void accept(Visitor *visitor);
        };

        // statements
        struct DefinitionStatement : Statement {
            explicit DefinitionStatement(Definition *definition);

            Definition *definition;

            void accept(Visitor *visitor);
        };

        struct ExpressionStatement : Statement {
            explicit ExpressionStatement(Expression *expression);

            Expression *expression;

            void accept(Visitor *visitor);
        };

        struct ImportStatement : Statement {
            ImportStatement(Token *token, StringLiteral *path);

            StringLiteral *path;

            void accept(Visitor *visitor);
        };

        // source file
        struct SourceFile : Node {
            SourceFile(Token *token, std::string name);

            std::string name;
            CodeBlock *code;
            std::vector<ImportStatement *> imports;

            void accept(Visitor *visitor);
        };

    }

}

#endif // ACORN_AST_NODES_H
