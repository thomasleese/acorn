//
// Created by Thomas Leese on 17/03/2016.
//

#ifndef ACORN_TOKEN_H
#define ACORN_TOKEN_H

#include <string>

namespace acorn {

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
            ThenKeyword,
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
            CCallKeyword,
            UsingKeyword,
            SizeofKeyword,
            StrideofKeyword,
            NewKeyword,
            InoutKeyword,
            ProtocolKeyword,

            // literals
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
            Operator,
            Identifier,
        };

        static std::string rule_string(Rule rule);

        std::string lexeme;
        Rule rule;

        std::string filename;
        std::string line;
        int lineNumber;
        int column;

    };

}

#endif // ACORN_TOKEN_H
