//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef JET_PARSER_H
#define JET_PARSER_H

#include <deque>
#include <map>
#include <string>

#include "Token.h"

namespace jet {
    namespace ast {
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
        struct RecordLiteral;
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
        struct Sizeof;
        struct Strideof;
        struct Parameter;
        struct VariableDefinition;
        struct FunctionDefinition;
        struct TypeDefinition;
        struct DefinitionStatement;
        struct ExpressionStatement;
        struct ImportStatement;
        struct SourceFile;
    }
}

using namespace jet;

class Parser {

public:
    explicit Parser(std::vector<Token *> tokens);
    ~Parser();

    ast::SourceFile *parse(std::string name);

private:
    void debug(std::string line);

    Token *readToken(Token::Rule rule);
    Token *skipToken(Token::Rule rule);
    bool isToken(Token::Rule rule) const;

    // misc
    ast::CodeBlock *readCodeBlock();

    // expressions
    ast::Expression *readExpression();
    ast::Identifier *readIdentifier(bool accept_parameters);
    ast::Identifier *readOperator(bool accept_parameters);
    ast::BooleanLiteral *readBooleanLiteral();
    ast::IntegerLiteral *readIntegerLiteral();
    ast::FloatLiteral *readFloatLiteral();
    ast::ImaginaryLiteral *readImaginaryLiteral();
    ast::StringLiteral *readStringLiteral();
    ast::SequenceLiteral *readSequenceLiteral();
    ast::MappingLiteral *readMappingLiteral();
    ast::RecordLiteral *readRecordLiteral();
    ast::Argument *readArgument();
    ast::Call *readCall(ast::Expression *operand);
    ast::CCall *readCCall();
    ast::Cast *readCast(ast::Expression *operand);
    ast::Selector *readSelector(ast::Expression *operand);
    ast::Index *readIndex(ast::Expression *operand);
    ast::While *readWhile();
    ast::For *readFor();
    ast::If *readIf();
    ast::Return *readReturn();
    ast::Spawn *readSpawn();
    ast::Sizeof *readSizeof();
    ast::Strideof *readStrideof();
    ast::Expression *readUnaryExpression();
    ast::Expression *readBinaryExpression(ast::Expression *lhs, int minPrecedence);
    ast::Expression *readPrimaryExpression();
    ast::Expression *readOperandExpression();

    // misc
    ast::Parameter *readParameter();

    // definitions
    ast::VariableDefinition *readVariableDefinition();
    ast::FunctionDefinition *readFunctionDefinition();
    ast::TypeDefinition *readTypeDefinition();

    // statements
    ast::Statement *readStatement();
    ast::ImportStatement *readImportStatement();

private:
    std::deque<Token *> m_tokens;
    std::map<std::string, int> m_operatorPrecendence;

};


#endif // JET_PARSER_H
