//
// Created by Thomas Leese on 17/03/2016.
//

#ifndef JET_TOKEN_H
#define JET_TOKEN_H

#include <string>

struct Token {

    enum Rule {
        Whitespace,
        Newline,
        Comment,
        EndOfFile,

        // keywords
        LetKeyword,
        DefKeyword,
        TypeKeyword,
        AsKeyword,
        EndKeyword,
        WhileKeyword,
        ForKeyword,
        InKeyword,
        IfKeyword,
        ElseKeyword,

        // literals
        BooleanLiteral,
        StringLiteral,
        FloatLiteral,
        ComplexLiteral,
        IntegerLiteral,

        // delimiters
        OpenBracket,
        CloseBracket,
        OpenParenthesis,
        CloseParenthesis,
        OpenBrace,
        CloseBrace,
        OpenChevron,
        CloseChevron,
        Comma,
        Dot,
        Colon,

        // identifiers
        Assignment,
        Identifier,
        Operator,
    };

    static std::string rule_string(Rule rule);

    std::string lexeme;
    Rule rule;

    std::string filename;
    std::string line;
    int lineNumber;
    int column;

};

#endif //JET_TOKEN_H
