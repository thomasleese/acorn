//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_ABSTRACTSYNTAXTREE_H
#define QUARK_ABSTRACTSYNTAXTREE_H

#include <map>
#include <queue>
#include <vector>

namespace Types {
    class Type;
}

namespace AST {

    struct Statement;
    struct Identifier;

    struct Node {
        virtual ~Node();

        virtual std::string pprint() const = 0;
    };

    struct CodeBlock : Node {
        std::vector<Statement *> statements;

        std::string pprint() const;
    };

    struct Module : Node {
        explicit Module(std::string name);

        std::string name;
        CodeBlock *code;

        std::string pprint() const;
    };

    struct TypeDeclaration : Node {
        Identifier *name;

        std::string pprint() const;
    };

    struct Parameter : Node {
        Identifier *name;
        TypeDeclaration *type;

        std::string pprint() const;
    };

    struct Expression : Node {

    };

    struct Identifier : Expression {
        std::string name;

        std::string pprint() const;
    };

    struct IntegerLiteral : Expression {
        int value;

        std::string pprint() const;
    };

    struct StringLiteral : Expression {
        std::string value;

        std::string pprint() const;
    };

    struct FunctionCall : Expression {
        Expression *operand;
        std::map<Identifier *, Expression *> arguments;

        std::string pprint() const;
    };

    struct Selector : Expression {
        Expression *operand;
        Identifier *name;

        std::string pprint() const;
    };

    struct Statement : Node {

    };

    struct LetStatement : Statement {
        Identifier *name;
        TypeDeclaration *type;
        Expression *expression;

        std::string pprint() const;
    };

    struct DefStatement : Statement {
        Identifier *name;
        std::vector<Parameter> parameters;
        CodeBlock code;
        TypeDeclaration *type;

        std::string pprint() const;
    };

    struct TypeStatement : Statement {
        Identifier *name;
        std::map<Identifier *, TypeDeclaration *> fields;

        std::string pprint() const;
    };

    struct ExpressionStatement : Statement {
        Expression *expression;

        std::string pprint() const;
    };

};

#endif //QUARK_ABSTRACTSYNTAXTREE_H
