//
// Created by Thomas Leese on 17/03/2016.
//

#pragma once

#include <string>

namespace acorn {

    namespace parser {

        struct Token {

            enum Kind {
                EndOfFile,
                Newline,
                Indent,
                Deindent,

                String,
                Float,
                Int,

                OpenBracket,
                CloseBracket,
                OpenParenthesis,
                CloseParenthesis,
                OpenBrace,
                CloseBrace,
                Comma,
                Dot,
                Colon,
                Semicolon,
                Assignment,

                Keyword,
                Operator,
                Name,
            };

            static std::string kind_to_string(const Kind &kind);

            std::string to_string() const;

            Kind kind;
            std::string lexeme;

            std::string filename;
            std::string line;
            int line_number;
            int column;

        };

        std::ostream &operator<<(std::ostream &stream, const Token &token);

    }

}
