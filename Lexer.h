//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_LEXER_H
#define QUARK_LEXER_H

#include <map>
#include <string>
#include <vector>

class Lexer {

public:
    enum Rule {
        Whitespace,
        Newline,

        // keywords
        LetKeyword,
        DefKeyword,
        TypeKeyword,
        AsKeyword,
        EndKeyword,

        // literals
        BooleanLiteral,
        IntegerLiteral,
        FloatLiteral,
        StringLiteral,
        ComplexLiteral,

        Identifier,
        Operator,

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
        Colon
    };

    struct Token {
        std::string lexeme;
        Rule rule;
    };

public:
    explicit Lexer();
    ~Lexer();

private:
    void loadRules();

public:
    std::vector<Token> tokenise(std::string filename) const;

private:
    std::map<Rule, std::string> m_rules;
};

#endif //QUARK_LEXER_H
