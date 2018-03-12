#include "catch.hpp"

#include "acorn/parser/token.h"

using namespace acorn::parser;

SCENARIO("checking if something is a keyword") {
    GIVEN("a string") {
        WHEN("it is a keyword") {
            REQUIRE(is_keyword("let"));
            REQUIRE(is_keyword("def"));
            REQUIRE(is_keyword("if"));
            REQUIRE(is_keyword("unless"));
        }

        WHEN("it is empty") {
            REQUIRE(!is_keyword(""));
        }

        WHEN("it is not a keyword") {
            REQUIRE(!is_keyword("not a keyword"));
            REQUIRE(!is_keyword("23891h91"));
        }
    }
}

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
