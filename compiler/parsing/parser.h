//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef ACORN_PARSER_H
#define ACORN_PARSER_H

#include <deque>
#include <map>
#include <string>

#include "token.h"

namespace acorn {

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
        class While;
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

    class Parser {

    public:
        Parser(diagnostics::Reporter *diagnostics, Lexer &lexer);
        ~Parser();

        ast::SourceFile *parse(std::string name);

    private:
        void debug(std::string line);

        bool next_token();
        bool read_token(Token::Kind kind, Token &token);
        bool skip_token(Token::Kind kind);
        bool is_token(Token::Kind kind);

        // misc
        ast::CodeBlock *readCodeBlock(bool in_switch = false);

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
        ast::CodeBlock *readFor();
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
        ast::ProtocolDefinition *readProtocolDefinition();
        ast::EnumDefinition *readEnumDefinition();

        // statements
        ast::Statement *readStatement();
        ast::ImportStatement *readImportStatement();

    private:
        diagnostics::Reporter *m_diagnostics;
        Lexer &m_lexer;
        std::deque<Token> m_tokens;
        std::map<std::string, int> m_operatorPrecendence;

    };

}

#endif // ACORN_PARSER_H
