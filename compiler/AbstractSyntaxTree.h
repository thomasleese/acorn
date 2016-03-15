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
    struct Misc : Node { };
    struct Statement : Node { };
    struct Definition : Node { };

    // expressions
    struct Identifier : Expression {
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

    struct FunctionCall : Expression {
        Expression *operand;
        std::map<Identifier *, Expression *> arguments;

        void accept(Visitor *visitor);
    };

    struct Selector : Expression {
        Expression *operand;
        Identifier *name;

        void accept(Visitor *visitor);
    };

    // misc
    struct TypeDeclaration : Misc {
        Identifier *name;

        void accept(Visitor *visitor);
    };

    struct Parameter : Misc {
        Identifier *name;
        TypeDeclaration *type;
        Expression *defaultExpression;

        void accept(Visitor *visitor);
    };

    struct CodeBlock : Misc {
        std::vector<Statement *> statements;

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
        std::vector<Parameter> parameters;
        CodeBlock code;
        TypeDeclaration *type;

        void accept(Visitor *visitor);
    };

    struct TypeDefinition : Definition {
        Identifier *name;
        std::map<Identifier *, TypeDeclaration *> fields;

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

        // expressions
        virtual void visit(Identifier *expression) = 0;
        virtual void visit(IntegerLiteral *expression) = 0;
        virtual void visit(StringLiteral *expression) = 0;
        virtual void visit(FunctionCall *expression) = 0;
        virtual void visit(Selector *expression) = 0;

        // misc
        virtual void visit(TypeDeclaration *type) = 0;
        virtual void visit(Parameter *parameter) = 0;
        virtual void visit(CodeBlock *block) = 0;

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
