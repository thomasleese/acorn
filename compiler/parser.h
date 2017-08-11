//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef ACORN_PARSER_H
#define ACORN_PARSER_H

#include <deque>
#include <map>
#include <string>

#include "ast/visitor.h"

namespace acorn {

    class Parser : public diagnostics::Reporter {

    public:
        explicit Parser(Lexer &lexer);
        ~Parser();

        std::unique_ptr<ast::SourceFile> parse(std::string name);

    private:
        void debug(std::string line);

        Token front_token();
        bool next_non_newline_token();
        void collapse_deindent_indent_tokens();
        bool next_token();
        bool fill_token();
        bool read_token(Token::Kind kind, Token &token);
        bool skip_token(Token::Kind kind);
        bool is_token_available();
        bool is_token(Token::Kind kind);
        bool is_and_skip_token(Token::Kind kind);
        bool skip_deindent_and_end_token();

        bool read_keyword(const std::string &name, Token &token);
        bool skip_keyword(const std::string &name);
        bool is_keyword(const std::string &name);
        bool is_and_skip_keyword(const std::string &name);

        std::unique_ptr<ast::Block> read_block(bool read_end = true);
        std::unique_ptr<ast::Expression> read_expression(bool parse_comma = true);
        std::unique_ptr<ast::Name> read_name_or_operator(Token::Kind kind, bool accept_parameters);
        std::unique_ptr<ast::Name> read_name(bool accept_parameters);
        std::unique_ptr<ast::Name> read_operator(bool accept_parameters);
        std::unique_ptr<ast::VariableDeclaration> read_variable_declaration();
        std::unique_ptr<ast::Int> read_int();
        std::unique_ptr<ast::Float> read_float();
        std::unique_ptr<ast::String> read_string();
        std::unique_ptr<ast::List> read_list();
        std::unique_ptr<ast::Dictionary> read_dictionary();
        std::unique_ptr<ast::Call> read_call(std::unique_ptr<ast::Expression> operand);
        std::unique_ptr<ast::CCall> read_ccall();
        std::unique_ptr<ast::Cast> read_cast(std::unique_ptr<ast::Expression> operand);
        std::unique_ptr<ast::Selector> read_selector(std::unique_ptr<ast::Expression> operand, bool allow_operators = false);
        std::unique_ptr<ast::Call> read_index(std::unique_ptr<ast::Expression> operand);
        std::unique_ptr<ast::While> read_while();
        std::unique_ptr<ast::Block> read_for();
        std::unique_ptr<ast::If> read_if();
        std::unique_ptr<ast::Return> read_return();
        std::unique_ptr<ast::Spawn> read_spawn();
        std::unique_ptr<ast::Case> read_case();
        std::unique_ptr<ast::Switch> read_switch();
        std::unique_ptr<ast::Expression> read_unary_expression(bool parse_comma);
        std::unique_ptr<ast::Expression> read_binary_expression(std::unique_ptr<ast::Expression> lhs, int min_precedence);
        std::unique_ptr<ast::Expression> read_parenthesis_expression();
        std::unique_ptr<ast::Expression> read_primary_expression();
        std::unique_ptr<ast::Expression> read_operand_expression(bool parse_comma);
        std::unique_ptr<ast::Parameter> read_parameter();
        std::unique_ptr<ast::Let> read_let();
        std::unique_ptr<ast::Selector> read_method_signature_name();
        std::unique_ptr<ast::Def> read_def();
        std::unique_ptr<ast::Type> read_type();
        std::unique_ptr<ast::Module> read_module();
        std::unique_ptr<ast::Import> read_import_expression();

    private:
        Lexer &m_lexer;
        std::deque<Token> m_tokens;
        std::map<std::string, int> m_operator_precendence;

    };

}

#endif // ACORN_PARSER_H
