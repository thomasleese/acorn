//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_PARSER_H
#define QUARK_PARSER_H

#include <deque>
#include <string>

namespace AST {
    struct Node;
    struct Expression;
    struct Misc;
    struct Statement;
    struct Definition;
    struct Identifier;
    struct IntegerLiteral;
    struct StringLiteral;
    struct FunctionCall;
    struct Selector;
    struct TypeDeclaration;
    struct Parameter;
    struct CodeBlock;
    struct VariableDefinition;
    struct FunctionDefinition;
    struct TypeDefinition;
    struct DefinitionStatement;
    struct ExpressionStatement;
    struct Module;
}

class Parser {

public:
    explicit Parser(std::vector<Lexer::Token> tokens);
    ~Parser();

    AST::Module *parse(std::string name);

private:
    void skipNewlines();
    void readNewlines();

    Lexer::Token readToken(Lexer::Rule rule);
    bool isToken(Lexer::Rule rule) const;

    // expressions
    AST::Expression *readExpression();
    AST::Identifier *readIdentifier();
    AST::Identifier *readOperator();
    AST::IntegerLiteral *readIntegerLiteral();
    AST::StringLiteral *readStringLiteral();
    AST::FunctionCall *readFunctionCall(AST::Expression *operand);
    AST::Selector *readSelector(AST::Expression *operand);
    AST::Expression *readUnaryExpression();
    AST::Expression *readBinaryExpression(AST::Expression *lhs, int minPrecedence);
    AST::Expression *readOperandExpression();

    // misc
    AST::TypeDeclaration *readTypeDeclaration();
    AST::Parameter readParameter();

    // definitions
    AST::VariableDefinition *readVariableDefinition();
    AST::FunctionDefinition *readFunctionDefinition();
    AST::TypeDefinition *readTypeDefinition();

    // statements
    AST::Statement *readStatement();

private:
    std::deque<Lexer::Token> m_tokens;
    std::map<std::string, int> m_operatorPrecendence;

};


#endif //QUARK_PARSER_H
