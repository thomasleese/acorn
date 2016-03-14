//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_ABSTRACTSYNTAXTREE_H
#define QUARK_ABSTRACTSYNTAXTREE_H

#include <map>
#include <queue>
#include <vector>

namespace AST {

    struct Statement;
    struct Identifier;

    struct CodeBlock {
        std::vector<Statement *> statements;
    };

    struct Parameter {
        Identifier *name;
        Identifier *type;
    };

    struct Expression {

    };

    struct Identifier : Expression {
        std::string name;
    };

    struct IntegerLiteral : Expression {
        int value;
    };

    struct StringLiteral : Expression {
        std::string value;
    };

    struct FunctionCall : Expression {
        Expression *operand;
        std::map<Identifier *, Expression *> arguments;
    };

    struct Selector : Expression {
        Expression *operand;
        Identifier *name;
    };

    struct Statement {

    };

    struct LetStatement : Statement {
        Identifier *name;
        Expression *expression;
    };

    struct DefStatement : Statement {
        Identifier *name;
        std::vector<Parameter> parameters;
        CodeBlock code;
    };

    struct TypeStatement : Statement {
        Identifier *name;
        std::map<Identifier *, Identifier *> fields;
    };

    struct ExpressionStatement : Statement {
        Expression *expression;
    };

};

#endif //QUARK_ABSTRACTSYNTAXTREE_H
