#include <iostream>
#include <vector>

#include <catch.hpp>

#include "acorn/ast/nodes.h"
#include "acorn/parser/scanner.h"

#include "acorn/parser/parser.h"

using namespace acorn::parser;

SCENARIO("parsing source code into an AST") {
    GIVEN("a string of source code") {
        WHEN("it is realistic") {
            std::string code =
                "def main(a as Int)\n"
                "  if a == 0\n"
                "    test(a)\n"
                "  end\n"
                "end\n";

            Scanner scanner(code, "realistic.acorn");
            Parser parser(scanner);

            auto source_file = parser.parse("realistic.acorn");

            THEN("it should parse") {
                REQUIRE(source_file != nullptr);
            }
        }
    }
}
