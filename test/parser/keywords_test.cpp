#include "catch.hpp"

#include "acorn/parser/keywords.h"

using acorn::parser::is_keyword;

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
