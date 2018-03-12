#include "catch.hpp"

#include "acorn/parser/token.h"

using namespace acorn::parser;

SCENARIO("getting the kind of a token") {
    GIVEN("a token kind") {
        REQUIRE(Token::kind_to_string(Token::Keyword) == "keyword");
        REQUIRE(Token::kind_to_string(Token::Name) == "name");
    }

    GIVEN("a token") {
        Token token;
        token.lexeme = "test";

        WHEN("it is a keyword") {
            token.kind = Token::Keyword;

            REQUIRE(token.kind_string() == "test");
        }

        WHEN("it is something else") {
            token.kind = Token::Name;

            REQUIRE(token.kind_string() == "name");
        }
    }
}
