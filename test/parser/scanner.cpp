#include <iostream>
#include <vector>

#include <catch.hpp>
#include <spdlog/spdlog.h>

#include "acorn/parser/scanner.h"

using namespace acorn::parser;

static auto logger = spdlog::get("acorn");

bool keywords_match(Scanner &scanner, std::vector<Token> &expected_tokens) {
    int i = 0;

    Token actual_token;
    for (auto &expected_token : expected_tokens) {
        if (!scanner.next_token(actual_token)) {
            return false;
        }

        if (expected_token != actual_token) {
            std::cerr << expected_token << " does not match " << actual_token << std::endl;
            return false;
        }
    }

    if (!scanner.next_token(actual_token)) {
        return false;
    }

    if (actual_token.kind != Token::EndOfFile) {
        std::cerr << actual_token << " is not an end of file" << std::endl;
        return false;
    }

    return true;
}

SCENARIO("scanner source code for tokens") {
    GIVEN("a string of source code") {
        WHEN("it is realistic") {
            std::string code =
                "def main(a as Int)\n"
                "  test(a)\n"
                "end\n";

            Scanner scanner(code, "realistic.acorn");

            std::vector<Token> tokens = {
                Token(Token::Keyword, "def"),
                Token(Token::Name, "main"),
                Token(Token::OpenParenthesis, "("),
                Token(Token::Name, "a"),
                Token(Token::Keyword, "as"),
                Token(Token::Name, "Int"),
                Token(Token::CloseParenthesis, ")"),
                Token(Token::Newline, "\n"),
                Token(Token::Indent),
                Token(Token::Name, "test"),
                Token(Token::OpenParenthesis, "("),
                Token(Token::Name, "a"),
                Token(Token::CloseParenthesis, ")"),
                Token(Token::Newline, "\n"),
                Token(Token::Deindent),
                Token(Token::Keyword, "end"),
                Token(Token::Newline, "\n"),
            };

            THEN("the keywords should match") {
                REQUIRE(keywords_match(scanner, tokens));
            }
        }

        WHEN("it has all kinds of tokens") {
            std::string code =
                "\n  \n'string' 10.5 9 "
                "[ ] ( ) { } , . : ; = "
                "let def type as while for in if else not and or end continue "
                "+ - * / abc";

            Scanner scanner(code, "all.acorn");

            std::vector<Token> tokens = {
                Token(Token::Newline, "\n"),
                Token(Token::Indent),
                Token(Token::Newline, "\n"),
                Token(Token::Deindent),
                Token(Token::String, "string"),
                Token(Token::Float, "10.5"),
                Token(Token::Int, "9"),
                Token(Token::OpenBracket, "["),
                Token(Token::CloseBracket, "]"),
                Token(Token::OpenParenthesis, "("),
                Token(Token::CloseParenthesis, ")"),
                Token(Token::OpenBrace, "{"),
                Token(Token::CloseBrace, "}"),
                Token(Token::Comma, ","),
                Token(Token::Dot, "."),
                Token(Token::Colon, ":"),
                Token(Token::Semicolon, ";"),
                Token(Token::Assignment, "="),
                Token(Token::Keyword, "let"),
                Token(Token::Keyword, "def"),
                Token(Token::Keyword, "type"),
                Token(Token::Keyword, "as"),
                Token(Token::Keyword, "while"),
                Token(Token::Keyword, "for"),
                Token(Token::Keyword, "in"),
                Token(Token::Keyword, "if"),
                Token(Token::Keyword, "else"),
                Token(Token::Keyword, "not"),
                Token(Token::Keyword, "and"),
                Token(Token::Keyword, "or"),
                Token(Token::Keyword, "end"),
                Token(Token::Keyword, "continue"),
                Token(Token::Operator, "+"),
                Token(Token::Operator, "-"),
                Token(Token::Operator, "*"),
                Token(Token::Operator, "/"),
                Token(Token::Name, "abc"),
            };

            THEN("the keywords should match") {
                REQUIRE(keywords_match(scanner, tokens));
            }
        }
    }
}
