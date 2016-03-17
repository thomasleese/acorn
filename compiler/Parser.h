//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_PARSER_H
#define QUARK_PARSER_H

#include <deque>
#include <string>

#include "Token.h"

namespace AST {
    struct Node;
    struct Expression;
    struct Misc;
    struct Statement;
    struct Definition;
    struct CodeBlock;
    struct Identifier;
    struct IntegerLiteral;
    struct FloatLiteral;
    struct StringLiteral;
    struct Argument;
    struct Call;
    struct Selector;
    struct While;
    struct For;
    struct If;
    struct TypeDeclaration;
    struct Parameter;
    struct VariableDefinition;
    struct FunctionDefinition;
    struct TypeDefinition;
    struct DefinitionStatement;
    struct ExpressionStatement;
    struct Module;
}

class Parser {

public:
    explicit Parser(std::vector<Token *> tokens);
    ~Parser();

    AST::Module *parse(std::string name);

private:
    void debug(std::string line);

    Token *readToken(Token::Rule rule);
    Token *skipToken(Token::Rule rule);
    bool isToken(Token::Rule rule) const;

    // misc
    AST::CodeBlock *readCodeBlock();

    // expressions
    AST::Expression *readExpression();
    AST::Identifier *readIdentifier();
    AST::Identifier *readOperator();
    AST::IntegerLiteral *readIntegerLiteral();
    AST::FloatLiteral *readFloatLiteral();
    AST::StringLiteral *readStringLiteral();
    AST::Argument *readArgument();
    AST::Call *readCall(AST::Expression *operand);
    AST::Selector *readSelector(AST::Expression *operand);
    AST::While *readWhile();
    AST::For *readFor();
    AST::If *readIf();
    AST::Expression *readUnaryExpression();
    AST::Expression *readBinaryExpression(AST::Expression *lhs, int minPrecedence);
    AST::Expression *readOperandExpression();

    // misc
    AST::TypeDeclaration *readTypeDeclaration();
    AST::Parameter *readParameter();

    // definitions
    AST::VariableDefinition *readVariableDefinition();
    AST::FunctionDefinition *readFunctionDefinition();
    AST::TypeDefinition *readTypeDefinition();

    // statements
    AST::Statement *readStatement();

private:
    std::deque<Token *> m_tokens;
    std::map<std::string, int> m_operatorPrecendence;

};


#endif //QUARK_PARSER_H
