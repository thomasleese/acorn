//
// Created by Thomas Leese on 17/03/2016.
//

#pragma once

#include <string>

namespace acorn::parser {

    bool is_keyword(const std::string &name);

    class SourceLocation {
    public:
        SourceLocation(std::string filename = "<unknown>", std::string line = "", int line_number = 0, int column = 0);

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
            Unknown,

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

        Token(Kind kind = Unknown, std::string lexeme = "");

        static std::string kind_to_string(const Kind &kind);

        std::string kind_string() const;
        std::string lexeme_string() const;

        bool operator==(const Token &token);
        bool operator!=(const Token &token);

    private:
        friend std::ostream &operator<<(std::ostream &stream, const Token &token);

    public:
        Kind kind;
        std::string lexeme;
        SourceLocation location;
    };

}
