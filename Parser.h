//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_PARSER_H
#define QUARK_PARSER_H

#include <deque>
#include <string>

namespace AST {
    struct CodeBlock;
    struct Parameter;
    struct Expression;
    struct Identifier;
    struct Literal;
    struct IntegerLiteral;
    struct StringLiteral;
    struct Statement;
    struct LetStatement;
    struct DefStatement;
    struct TypeStatement;
    struct ExpressionStatement;
    struct FunctionCall;
    struct Selector;
}

class Parser {

public:
    explicit Parser(std::vector<Lexer::Token> tokens);
    ~Parser();

    AST::CodeBlock parse();

private:
    void skipNewlines();
    void readNewlines();

    Lexer::Token readToken(Lexer::Rule rule);
    bool isToken(Lexer::Rule rule) const;

    AST::Identifier *readIdentifier();
    bool isIdentifier() const;
    AST::Identifier *readOperator();
    bool isOperator() const;

    AST::Parameter readParameter();

    AST::Expression *readExpression();
    AST::Expression *readUnaryExpression();
    AST::Expression *readBinaryExpression(AST::Expression *lhs, int minPrecedence);
    AST::Expression *readOperandExpression();
    AST::IntegerLiteral *readIntegerLiteral();
    AST::StringLiteral *readStringLiteral();
    AST::FunctionCall *readFunctionCall(AST::Expression *operand);
    AST::Selector *readSelector(AST::Expression *operand);

    AST::Statement *readStatement();
    AST::LetStatement *readLetStatement();
    AST::DefStatement *readDefStatement();
    AST::TypeStatement *readTypeStatement();
    AST::ExpressionStatement *readExpressionStatement();

private:
    std::deque<Lexer::Token> m_tokens;
    std::map<std::string, int> m_operatorPrecendence;

};


#endif //QUARK_PARSER_H
