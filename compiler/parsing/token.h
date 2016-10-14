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
            RepeatKeyword,
            UnlessKeyword,
            MutableKeyword,
            SpawnKeyword,
            CCallKeyword,
            UsingKeyword,
            NewKeyword,
            InoutKeyword,
            ProtocolKeyword,
            EnumKeyword,
            SwitchKeyword,
            CaseKeyword,
            DefaultKeyword,

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

        Token(Kind kind, std::string lexeme);

        /**
         * Get a human-readable string representation of a kind of token.
         */
        static std::string kind_string(Kind kind);

        Kind kind;
        std::string lexeme;

        std::string filename;
        std::string line;
        int line_number;
        int column;

    };

}
