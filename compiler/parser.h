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

        Token front_token();
        bool next_token();
        bool fill_token();
        bool read_token(Token::Kind kind, Token &token);
        bool skip_token(Token::Kind kind);
        bool is_token_available();
        bool is_token(Token::Kind kind);
        bool is_and_skip_token(Token::Kind kind);
        bool skip_deindent_and_end_token();

        ast::Block *read_block(bool in_switch = false);
        ast::Expression *read_expression(bool parse_comma = true);
        ast::Name *read_name_or_operator(Token::Kind kind, bool accept_parameters);
        ast::Name *read_name(bool accept_parameters);
        ast::Name *read_operator(bool accept_parameters);
        ast::VariableDeclaration *read_variable_declaration();
        ast::IntegerLiteral *read_integer_literal();
        ast::FloatLiteral *read_float_literal();
        ast::StringLiteral *read_string_literal();
        ast::SequenceLiteral *read_sequence_literal();
        ast::MappingLiteral *read_mapping_literal();
        ast::RecordLiteral *read_record_literal();
        ast::Call *read_call(ast::Expression *operand);
        ast::CCall *read_ccall();
        ast::Cast *read_cast(ast::Expression *operand);
        ast::Selector *read_selector(ast::Expression *operand);
        ast::Call *read_index(ast::Expression *operand);
        ast::While *read_while();
        ast::Block *read_for();
        ast::If *read_if();
        ast::Return *read_return();
        ast::Spawn *read_spawn();
        ast::Case *read_case();
        ast::Switch *read_switch();
        ast::Expression *read_unary_expression(bool parse_comma);
        ast::Expression *read_binary_expression(ast::Expression *lhs, int min_precedence);
        ast::Expression *read_parenthesis_expression();
        ast::Expression *read_primary_expression();
        ast::Expression *read_operand_expression(bool parse_comma);
        ast::Parameter *read_parameter();
        ast::VariableDefinition *read_variable_definition();
        ast::FunctionDefinition *read_function_definition();
        ast::TypeDefinition *read_type_definition();
        ast::Import *read_import_expression();

    private:
        Lexer &m_lexer;
        std::deque<Token> m_tokens;
        std::map<std::string, int> m_operator_precendence;

    };

}

#endif // ACORN_PARSER_H
