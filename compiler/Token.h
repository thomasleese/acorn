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
        AndKeyword,
        OrKeyword,
        NotKeyword,
        ContinueKeyword,
        BreakKeyword,
        TryKeyword,
        ExceptKeyword,
        RaiseKeyword,
        FinallyKeyword,
        FromKeyword,
        ImportKeyword,
        ReturnKeyword,
        WithKeyword,
        YieldKeyword,
        AsyncKeyword,
        DoKeyword,
        UnlessKeyword,
        MutableKeyword,
        SpawnKeyword,

        // literals
        BooleanLiteral,
        StringLiteral,
        ImaginaryLiteral,
        FloatLiteral,
        IntegerLiteral,

        // delimiters
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

#endif // JET_TOKEN_H
