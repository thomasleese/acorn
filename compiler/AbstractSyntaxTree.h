//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef JET_ABSTRACTSYNTAXTREE_H
#define JET_ABSTRACTSYNTAXTREE_H

#include <string>
#include <vector>
#include "Types.h"

struct Token;

namespace AST {

    class Visitor;

    struct Node {
        explicit Node(Token *token);
        virtual ~Node();

        virtual void accept(Visitor *visitor) = 0;

        Token *token;
        Types::Type *type;
    };

    // basic categories
    struct Expression : Node {
        using Node::Node;
    };

    struct Statement : Node {
        using Node::Node;
    };

    struct Definition : Node {
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
        explicit Identifier(Token *token);
        Identifier(Token *token, std::string name);

        std::string name;

        void accept(Visitor *visitor);
    };

    struct BooleanLiteral : Expression {
        using Expression::Expression;

        bool value;

        void accept(Visitor *visitor);
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

    struct Argument : Node {
        explicit Argument(Token *token);
        Argument(Token *token, std::string name);

        Identifier *name;
        Expression *value;

        void accept(Visitor *visitor);
    };

    struct Call : Expression {
        Call(Token *token);
        Call(std::string name, Token *token);

        Expression *operand;
        std::vector<Argument *> arguments;

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

    struct Type : Expression {
        Type(Token *token);
        Type(std::string name, Token *token);

        Identifier *name;
        std::vector<Type *> parameters;

        void accept(Visitor *visitor);
    };

    struct Cast : Expression {
        Cast(Token *token);
        Cast(Type *typeNode, Token *token);

        Type *typeNode;

        void accept(Visitor *visitor);
    };

    // misc
    struct Parameter : Node {
        explicit Parameter(Token *token);

        Identifier *name;
        Cast *cast;
        Expression *defaultExpression;

        void accept(Visitor *visitor);
    };

    // definitions
    struct VariableDefinition : Definition {
        VariableDefinition(Token *token);
        VariableDefinition(std::string name, Token *token);

        bool is_mutable;
        Identifier *name;
        Cast *cast;
        Expression *expression;

        void accept(Visitor *visitor);
    };

    struct FunctionDefinition : Definition {
        using Definition::Definition;

        Identifier *name;
        std::vector<Parameter *> parameters;
        CodeBlock *code;
        Cast *returnCast;

        void accept(Visitor *visitor);
    };

    struct TypeDefinition : Definition {
        using Definition::Definition;

        Type *name;
        Type *alias;
        std::vector<Parameter *> fields;

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

    // module
    struct Module : Node {
        explicit Module(Token *token, std::string name);

        std::string name;
        CodeBlock *code;

        void accept(Visitor *visitor);
    };

    // visitor
    class Visitor {
    public:
        virtual ~Visitor();

        // misc
        virtual void visit(CodeBlock *block) = 0;

        // expressions
        virtual void visit(Identifier *expression) = 0;
        virtual void visit(BooleanLiteral *boolean) = 0;
        virtual void visit(IntegerLiteral *expression) = 0;
        virtual void visit(FloatLiteral *expression) = 0;
        virtual void visit(ImaginaryLiteral *expression) = 0;
        virtual void visit(StringLiteral *expression) = 0;
        virtual void visit(SequenceLiteral *expression) = 0;
        virtual void visit(MappingLiteral *expression) = 0;
        virtual void visit(Argument *expression) = 0;
        virtual void visit(Call *expression) = 0;
        virtual void visit(Assignment *expression) = 0;
        virtual void visit(Selector *expression) = 0;
        virtual void visit(Comma *expression) = 0;
        virtual void visit(While *expression) = 0;
        virtual void visit(For *expression) = 0;
        virtual void visit(If *expression) = 0;
        virtual void visit(Type *type) = 0;
        virtual void visit(Cast *type) = 0;

        // misc
        virtual void visit(Parameter *parameter) = 0;

        // definitions
        virtual void visit(VariableDefinition *definition) = 0;
        virtual void visit(FunctionDefinition *definition) = 0;
        virtual void visit(TypeDefinition *definition) = 0;

        // statements
        virtual void visit(DefinitionStatement *statement) = 0;
        virtual void visit(ExpressionStatement *statement) = 0;

        // module
        virtual void visit(Module *module) = 0;
    };

    class Simplifier : public Visitor {
        void visit(CodeBlock *block);

        void visit(Identifier *expression);
        void visit(BooleanLiteral *boolean);
        void visit(IntegerLiteral *expression);
        void visit(FloatLiteral *expression);
        void visit(ImaginaryLiteral *expression);
        void visit(StringLiteral *expression);
        void visit(SequenceLiteral *expression);
        void visit(MappingLiteral *expression);
        void visit(Argument *expression);
        void visit(Call *expression);
        void visit(Assignment *expression);
        void visit(Selector *expression);
        void visit(Comma *expression);
        void visit(While *expression);
        void visit(For *expression);
        void visit(If *expression);
        void visit(Type *type);
        void visit(Cast *type);

        void visit(Parameter *parameter);

        void visit(VariableDefinition *definition);
        void visit(FunctionDefinition *definition);
        void visit(TypeDefinition *definition);

        void visit(DefinitionStatement *statement);
        void visit(ExpressionStatement *statement);

        void visit(Module *module);

    private:
        std::vector<Statement *> m_insertStatements;
        bool m_removeStatement;
    };

};

#endif // JET_ABSTRACTSYNTAXTREE_H
