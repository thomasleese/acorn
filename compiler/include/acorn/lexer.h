//
// Created by Thomas Leese on 13/03/2016.
//

#pragma once

#include <deque>
#include <fstream>
#include <vector>

#include "diagnostics.h"
#include "token.h"

namespace acorn {

    /**
     * Lexical Analysis. Takes source code input and provides a stream of
     * tokens as output.
     */
    class Lexer : public diagnostics::Reporter {
    public:
        Lexer(std::string filename);

        /**
         * Read the next token.
         */
        bool next_token(Token &token);

        /**
         * Debug the lexer by printing all the tokens.
         */
        void debug();

    private:
        int get();
        void unget();

        Token make_token(Token::Kind kind = Token::EndOfFile) const;

        unsigned int skip_whitespace();
        void skip_comment();

        void next_line();
        void update_current_line();

        void update_indentation(Token &token);

        bool read_name(Token &token);
        bool read_keyword(Token &token) const;
        bool read_number(Token &token);
        bool read_string(Token &token);
        bool read_delimiter(Token &token);
        bool read_operator(Token &token);

    private:
        std::string m_data;
        std::deque<int> m_indentation;
        std::deque<Token> m_token_buffer;
        int m_pos;

        std::string m_filename;
        int m_current_column;
        int m_current_line_number;
        std::string m_current_line;
    };

}
