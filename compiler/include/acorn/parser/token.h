//
// Created by Thomas Leese on 17/03/2016.
//

#pragma once

#include <string>

namespace acorn::parser {

    class SourceLocation {
    public:
        std::string to_string() const;

    private:
        friend std::ostream &operator<<(std::ostream &stream, const SourceLocation &source_location);

    public:
        std::string filename;
        std::string line;
        int line_number;
        int column;
    };

    class Token {
    public:
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

        std::string kind_string() const;
        std::string to_string() const;

    private:
        friend std::ostream &operator<<(std::ostream &stream, const Token &token);

    public:
        Kind kind;
        std::string lexeme;
        SourceLocation location;
    };

}
