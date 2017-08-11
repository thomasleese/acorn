//
// Created by Thomas Leese on 17/03/2016.
//

#pragma once

#include <string>

namespace acorn {

    /**
     * A token of source code.
     */
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

        /**
         * Get a human-readable string representation of a kind of token.
         */
        static std::string as_string(Kind kind);

        Kind kind;
        std::string lexeme;

        std::string filename;
        std::string line;
        int line_number;
        int column;

    };

}
