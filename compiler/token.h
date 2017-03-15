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
            // source code elements
            EndOfFile,
            Newline,
            Indent,
            Deindent,

            // keywords
            LetKeyword,
            DefKeyword,
            TypeKeyword,
            AsKeyword,
            WhileKeyword,
            ForKeyword,
            InKeyword,
            IfKeyword,
            ElseKeyword,
            EndKeyword,
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
            Assignment,
            Arrow,

            // identifiers
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
