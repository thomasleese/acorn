//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef ACORN_PARSER_H
#define ACORN_PARSER_H

#include <deque>
#include <map>
#include <string>

#include "ast.h"

namespace acorn {

    class Parser : public diagnostics::Reporter {

    public:
        explicit Parser(Lexer &lexer);
        ~Parser();

        ast::SourceFile *parse(std::string name);

    private:
        void debug(std::string line);

        bool next_token();
        bool fill_token();
        bool read_token(Token::Kind kind, Token &token);
        bool skip_token(Token::Kind kind);
        bool is_token(Token::Kind kind);

        // misc
        ast::Block *readBlock(bool in_switch = false);

        // expressions
        ast::Expression *readExpression(bool parse_comma);
        ast::Identifier *readIdentifier(bool accept_parameters);
        ast::Identifier *readOperator(bool accept_parameters);
        ast::VariableDeclaration *readVariableDeclaration();
        ast::IntegerLiteral *readIntegerLiteral();
        ast::FloatLiteral *readFloatLiteral();
        ast::StringLiteral *readStringLiteral();
        ast::SequenceLiteral *readSequenceLiteral();
        ast::MappingLiteral *readMappingLiteral();
        ast::RecordLiteral *readRecordLiteral();
        ast::Call *readCall(ast::Expression *operand);
        ast::CCall *readCCall();
        ast::Cast *readCast(ast::Expression *operand);
        ast::Selector *readSelector(ast::Expression *operand);
        ast::Call *readIndex(ast::Expression *operand);
        ast::While *readWhile();
        ast::Block *readFor();
        ast::If *readIf();
        ast::Return *readReturn();
        ast::Spawn *readSpawn();
        ast::Case *readCase();
        ast::Switch *readSwitch();
        ast::Expression *readUnaryExpression(bool parse_comma);
        ast::Expression *readBinaryExpression(ast::Expression *lhs, int minPrecedence);
        ast::Expression *readPrimaryExpression();
        ast::Expression *readOperandExpression(bool parse_comma);

        // misc
        ast::Parameter *readParameter();

        // definitions
        ast::VariableDefinition *readVariableDefinition();
        ast::FunctionDefinition *readFunctionDefinition();
        ast::TypeDefinition *readTypeDefinition();

        // statements
        ast::Expression *readExpression();
        ast::ImportExpression *readImportExpression();

    private:
        Lexer &m_lexer;
        std::deque<Token> m_tokens;
        std::map<std::string, int> m_operatorPrecendence;

    };

}

#endif // ACORN_PARSER_H
