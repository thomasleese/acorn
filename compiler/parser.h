//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef ACORN_PARSER_H
#define ACORN_PARSER_H

#include <deque>
#include <map>
#include <string>

#include "compiler/pass.h"
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

    class Parser : public compiler::Pass {

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
        ast::Expression *readExpression(bool parse_comma);
        ast::Identifier *readIdentifier(bool accept_parameters);
        ast::Identifier *readOperator(bool accept_parameters);
        ast::VariableDeclaration *readVariableDeclaration();
        ast::IntegerLiteral *readIntegerLiteral();
        ast::FloatLiteral *readFloatLiteral();
        ast::ImaginaryLiteral *readImaginaryLiteral();
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
        ast::Sizeof *readSizeof();
        ast::Strideof *readStrideof();
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

        // statements
        ast::Statement *readStatement();
        ast::ImportStatement *readImportStatement();

    private:
        std::deque<Token *> m_tokens;
        std::map<std::string, int> m_operatorPrecendence;

    };

}

#endif // ACORN_PARSER_H
