#pragma once

#include <deque>

#include "../diagnostics.h"
#include "token.h"

namespace acorn::parser {

    class Scanner : public diagnostics::Reporter {
    public:
        Scanner(std::string filename);
        Scanner(std::istream &stream, std::string filename);
        Scanner(std::string data, std::string filename);

    private:
        void initialise_with_data(std::string data);

    public:
        bool next_token(Token &token);

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
        diagnostics::Logger m_logger;

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
