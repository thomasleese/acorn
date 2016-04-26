//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef JET_ABSTRACTSYNTAXTREE_H
#define JET_ABSTRACTSYNTAXTREE_H

#include <string>
#include <vector>

struct Token;

namespace Types {
    class Type;
}

namespace AST {

    class Visitor;

    struct Node {
        explicit Node(Token *token);
        virtual ~Node();

        virtual void accept(Visitor *visitor) = 0;
        virtual Node *clone() const = 0;

        Token *token;
        Types::Type *type;
    };

    // basic categories
    struct Expression : Node {
        using Node::Node;

        virtual Expression *clone() const = 0;
    };

    struct Statement : Node {
        using Node::Node;

        virtual Statement *clone() const = 0;
    };

    // misc
    struct CodeBlock : Node {
        using Node::Node;

        std::vector<Statement *> statements;

        void accept(Visitor *visitor);
        CodeBlock *clone() const;
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
        Identifier *clone() const;
    };

    struct Definition : Node {
        using Node::Node;

        Identifier *name;

        virtual Definition *clone() const = 0;
    };

    struct BooleanLiteral : Expression {
        using Expression::Expression;

        std::string value;

        void accept(Visitor *visitor);
        BooleanLiteral *clone() const;
    };

    struct IntegerLiteral : Expression {
        using Expression::Expression;

        std::string value;

        void accept(Visitor *visitor);
        IntegerLiteral *clone() const;
    };

    struct FloatLiteral : Expression {
        using Expression::Expression;

        std::string value;

        void accept(Visitor *visitor);
        FloatLiteral *clone() const;
    };

    struct ImaginaryLiteral : Expression {
        using Expression::Expression;

        std::string value;

        void accept(Visitor *visitor);
        ImaginaryLiteral *clone() const;
    };

    struct StringLiteral : Expression {
        using Expression::Expression;

        std::string value;

        void accept(Visitor *visitor);
        StringLiteral *clone() const;
    };

    struct SequenceLiteral : Expression {
        using Expression::Expression;

        std::vector<Expression *> elements;

        void accept(Visitor *visitor);
        SequenceLiteral *clone() const;
    };

    struct MappingLiteral : Expression {
        using Expression::Expression;

        std::vector<Expression *> keys;
        std::vector<Expression *> values;

        void accept(Visitor *visitor);
        MappingLiteral *clone() const;
    };

    struct Argument : Node {
        explicit Argument(Token *token);
        Argument(Token *token, std::string name);

        Identifier *name;
        Expression *value;

        void accept(Visitor *visitor);
        Argument *clone() const;
    };

    struct Call : Expression {
        Call(Token *token);
        Call(std::string name, Token *token);

        Expression *operand;
        std::vector<Argument *> arguments;

        void accept(Visitor *visitor);
        Call *clone() const;
    };

    struct CCall : Expression {
        CCall(Token *token);

        Identifier *name;
        std::vector<Identifier *> parameters;
        Identifier *returnType;
        std::vector<Expression *> arguments;

        void accept(Visitor *visitor);
        CCall *clone() const;
    };

    struct Assignment : Expression {
        explicit Assignment(Token *token, Identifier *lhs, Expression *rhs);

        Identifier *lhs;
        Expression *rhs;

        void accept(Visitor *visitor);
        Assignment *clone() const;
    };

    struct Selector : Expression {
        using Expression::Expression;

        Expression *operand;
        Identifier *name;

        void accept(Visitor *visitor);
        Selector *clone() const;
    };

    struct Index : Expression {
        using Expression::Expression;

        Expression *operand;
        Expression *index;

        void accept(Visitor *visitor);
        Index *clone() const;
    };

    struct Comma : Expression {
        Comma(Token *token);
        Comma(Expression *lhs, Expression *rhs, Token *token);

        Expression *lhs;
        Expression *rhs;

        void accept(Visitor *visitor);
        Comma *clone() const;
    };

    struct While : Expression {
        using Expression::Expression;

        Expression *condition;
        CodeBlock *code;

        void accept(Visitor *visitor);
        While *clone() const;
    };

    struct For : Expression {
        using Expression::Expression;

        Identifier *name;
        Expression *iterator;
        CodeBlock *code;

        void accept(Visitor *visitor);
        For *clone() const;
    };

    struct If : Expression {
        using Expression::Expression;

        Expression *condition;
        CodeBlock *trueCode;
        CodeBlock *falseCode;

        void accept(Visitor *visitor);
        If *clone() const;
    };

    struct Return : Expression {
        using Expression::Expression;

        Expression *expression;

        void accept(Visitor *visitor);
        Return *clone() const;
    };

    struct Spawn : Expression {
        Spawn(Token *token, Call *call);

        Call *call;

        void accept(Visitor *visitor);
        Spawn *clone() const;
    };

    // misc
    struct Parameter : Node {
        explicit Parameter(Token *token);

        Identifier *name;
        Identifier *typeNode;
        Expression *defaultExpression;

        void accept(Visitor *visitor);
        Parameter *clone() const;
    };

    // definitions
    struct VariableDefinition : Definition {
        VariableDefinition(Token *token);
        VariableDefinition(std::string name, Token *token);

        Identifier *typeNode;
        Expression *expression;

        void accept(Visitor *visitor);
        VariableDefinition *clone() const;
    };

    struct FunctionDefinition : Definition {
        using Definition::Definition;

        std::vector<Parameter *> parameters;
        CodeBlock *code;
        Identifier *returnType;

        void accept(Visitor *visitor);
        FunctionDefinition *clone() const;
    };

    struct TypeDefinition : Definition {
        TypeDefinition(Token *token);

        Identifier *alias;
        std::vector<Parameter *> fields;

        void accept(Visitor *visitor);
        TypeDefinition *clone() const;
    };

    // statements
    struct DefinitionStatement : Statement {
        explicit DefinitionStatement(Definition *definition);

        Definition *definition;

        void accept(Visitor *visitor);
        DefinitionStatement *clone() const;
    };

    struct ExpressionStatement : Statement {
        explicit ExpressionStatement(Expression *expression);

        Expression *expression;

        void accept(Visitor *visitor);
        ExpressionStatement *clone() const;
    };

    struct ImportStatement : Statement {
        ImportStatement(Token *token, StringLiteral *path);

        StringLiteral *path;

        void accept(Visitor *visitor);
        ImportStatement *clone() const;
    };

    // source file
    struct SourceFile : Node {
        SourceFile(Token *token, std::string name);

        std::string name;
        CodeBlock *code;
        std::vector<ImportStatement *> imports;

        void accept(Visitor *visitor);
        SourceFile *clone() const;
    };

    // visitor
    class Visitor {
    public:
        virtual ~Visitor();

        // misc
        virtual void visit(CodeBlock *block) = 0;

        // expressions
        virtual void visit(Identifier *identifier) = 0;
        virtual void visit(BooleanLiteral *boolean) = 0;
        virtual void visit(IntegerLiteral *expression) = 0;
        virtual void visit(FloatLiteral *expression) = 0;
        virtual void visit(ImaginaryLiteral *expression) = 0;
        virtual void visit(StringLiteral *expression) = 0;
        virtual void visit(SequenceLiteral *expression) = 0;
        virtual void visit(MappingLiteral *expression) = 0;
        virtual void visit(Argument *expression) = 0;
        virtual void visit(Call *expression) = 0;
        virtual void visit(CCall *expression) = 0;
        virtual void visit(Assignment *expression) = 0;
        virtual void visit(Selector *expression) = 0;
        virtual void visit(Index *expression) = 0;
        virtual void visit(Comma *expression) = 0;
        virtual void visit(While *expression) = 0;
        virtual void visit(For *expression) = 0;
        virtual void visit(If *expression) = 0;
        virtual void visit(Return *expression) = 0;
        virtual void visit(Spawn *expression) = 0;

        // misc
        virtual void visit(Parameter *parameter) = 0;

        // definitions
        virtual void visit(VariableDefinition *definition) = 0;
        virtual void visit(FunctionDefinition *definition) = 0;
        virtual void visit(TypeDefinition *definition) = 0;

        // statements
        virtual void visit(DefinitionStatement *statement) = 0;
        virtual void visit(ExpressionStatement *statement) = 0;
        virtual void visit(ImportStatement *statement) = 0;

        // module
        virtual void visit(SourceFile *module) = 0;
    };

};

#endif // JET_ABSTRACTSYNTAXTREE_H
