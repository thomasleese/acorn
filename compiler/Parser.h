//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef JET_PARSER_H
#define JET_PARSER_H

#include <deque>
#include <map>
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
    struct BooleanLiteral;
    struct IntegerLiteral;
    struct FloatLiteral;
    struct ImaginaryLiteral;
    struct StringLiteral;
    struct SequenceLiteral;
    struct MappingLiteral;
    struct Argument;
    struct Call;
    struct CCall;
    struct Cast;
    struct Selector;
    struct Index;
    struct While;
    struct For;
    struct If;
    struct Return;
    struct Spawn;
    struct Type;
    struct Parameter;
    struct VariableDefinition;
    struct FunctionDefinition;
    struct TypeDefinition;
    struct DefinitionStatement;
    struct ExpressionStatement;
    struct ImportStatement;
    struct SourceFile;
}

class Parser {

public:
    explicit Parser(std::vector<Token *> tokens);
    ~Parser();

    AST::SourceFile *parse(std::string name);

private:
    void debug(std::string line);

    Token *readToken(Token::Rule rule);
    Token *skipToken(Token::Rule rule);
    bool isToken(Token::Rule rule) const;

    // misc
    AST::CodeBlock *readCodeBlock();

    // expressions
    AST::Expression *readExpression();
    AST::Identifier *readIdentifier(bool accept_parameters);
    AST::Identifier *readOperator(bool accept_parameters);
    AST::BooleanLiteral *readBooleanLiteral();
    AST::IntegerLiteral *readIntegerLiteral();
    AST::FloatLiteral *readFloatLiteral();
    AST::ImaginaryLiteral *readImaginaryLiteral();
    AST::StringLiteral *readStringLiteral();
    AST::SequenceLiteral *readSequenceLiteral();
    AST::MappingLiteral *readMappingLiteral();
    AST::Argument *readArgument();
    AST::Call *readCall(AST::Expression *operand);
    AST::CCall *readCCall();
    AST::Cast *readCast(AST::Expression *operand);
    AST::Selector *readSelector(AST::Expression *operand);
    AST::Index *readIndex(AST::Expression *operand);
    AST::While *readWhile();
    AST::For *readFor();
    AST::If *readIf();
    AST::Return *readReturn();
    AST::Spawn *readSpawn();
    AST::Expression *readUnaryExpression();
    AST::Expression *readBinaryExpression(AST::Expression *lhs, int minPrecedence);
    AST::Expression *readPrimaryExpression();
    AST::Expression *readOperandExpression();

    // misc
    AST::Parameter *readParameter();

    // definitions
    AST::VariableDefinition *readVariableDefinition();
    AST::FunctionDefinition *readFunctionDefinition();
    AST::TypeDefinition *readTypeDefinition();

    // statements
    AST::Statement *readStatement();
    AST::ImportStatement *readImportStatement();

private:
    std::deque<Token *> m_tokens;
    std::map<std::string, int> m_operatorPrecendence;

};


#endif // JET_PARSER_H
