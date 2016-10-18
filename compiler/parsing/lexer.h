//
// Created by Thomas Leese on 13/03/2016.
//

#pragma once

#include <deque>
#include <fstream>
#include <vector>

#include "token.h"

namespace acorn {

    namespace diagnostics {
        class Diagnostics;
    }

    /**
     * Lexical Analysis. Takes source code input and provides a stream of
     * tokens as output.
     */
    class Lexer {
    public:
        Lexer(diagnostics::Diagnostics *diagnostics, std::string filename);

        /**
         * Read the next token.
         */
        bool next_token(Token &token);

    private:
        Token make_token(Token::Kind kind = Token::EndOfFile) const;

        unsigned int skip_whitespace();
        void skip_comment();

        void next_line();
        void update_indentation(Token &token);

        bool read_identifier(Token &token);
        bool read_keyword(Token &token) const;
        bool read_number(Token &token);
        bool read_string(Token &token);
        bool read_operator(Token &token);
        bool read_delimiter(Token &token);
        bool read_line_continuation(Token &token);

    private:
        diagnostics::Diagnostics *m_diagnostics;

        std::ifstream m_stream;
        std::deque<int> m_indentation;
        std::deque<Token> m_token_buffer;

        std::string m_filename;
        int m_current_column;
        int m_current_line_number;
        std::string m_current_line;
    };

}
