//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_ABSTRACTSYNTAXTREE_H
#define QUARK_ABSTRACTSYNTAXTREE_H

#include <map>
#include <queue>
#include <vector>

// definitions

// expressions

// statements

// module

namespace AST {

    class Visitor;

    struct Node {
        virtual ~Node();

        virtual void accept(Visitor *visitor) = 0;
    };

    // basic categories
    struct Expression : Node { };
    struct Statement : Node { };
    struct Definition : Node { };

    // misc
    struct CodeBlock : Node {
        std::vector<Statement *> statements;

        void accept(Visitor *visitor);
    };

    // expressions
    struct Identifier : Expression {
        explicit Identifier();
        Identifier(std::string name);

        std::string name;

        void accept(Visitor *visitor);
    };

    struct IntegerLiteral : Expression {
        int value;

        void accept(Visitor *visitor);
    };

    struct StringLiteral : Expression {
        std::string value;

        void accept(Visitor *visitor);
    };

    struct Argument : Node {
        explicit Argument();
        Argument(std::string name);

        Identifier *name;
        Expression *value;

        void accept(Visitor *visitor);
    };

    struct Call : Expression {
        Expression *operand;
        std::vector<Argument *> arguments;

        void accept(Visitor *visitor);
    };

    struct Selector : Expression {
        Expression *operand;
        Identifier *name;

        void accept(Visitor *visitor);
    };

    struct While : Expression {
        Expression *condition;
        CodeBlock *code;

        void accept(Visitor *visitor);
    };

    struct For : Expression {
        Identifier *name;
        Expression *iterator;
        CodeBlock *code;

        void accept(Visitor *visitor);
    };

    struct If : Expression {
        Expression *condition;
        CodeBlock *trueCode;
        CodeBlock *falseCode;

        void accept(Visitor *visitor);
    };

    // misc
    struct TypeDeclaration : Node {
        Identifier *name;

        void accept(Visitor *visitor);
    };

    struct Parameter : Node {
        Identifier *name;
        TypeDeclaration *type;
        Expression *defaultExpression;

        void accept(Visitor *visitor);
    };

    // definitions
    struct VariableDefinition : Definition {
        Identifier *name;
        TypeDeclaration *type;
        Expression *expression;

        void accept(Visitor *visitor);
    };

    struct FunctionDefinition : Definition {
        Identifier *name;
        std::vector<Parameter *> parameters;
        CodeBlock code;
        TypeDeclaration *type;

        void accept(Visitor *visitor);
    };

    struct TypeDefinition : Definition {
        Identifier *name;
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
        explicit Module(std::string name);

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
        virtual void visit(IntegerLiteral *expression) = 0;
        virtual void visit(StringLiteral *expression) = 0;
        virtual void visit(Argument *expression) = 0;
        virtual void visit(Call *expression) = 0;
        virtual void visit(Selector *expression) = 0;
        virtual void visit(While *expression) = 0;
        virtual void visit(For *expression) = 0;
        virtual void visit(If *expression) = 0;

        // misc
        virtual void visit(TypeDeclaration *type) = 0;
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

};

#endif //QUARK_ABSTRACTSYNTAXTREE_H
