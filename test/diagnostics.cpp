#include <catch.hpp>

#include "acorn/diagnostics.h"

using namespace acorn::parser;
using namespace acorn::diagnostics;

SCENARIO("logging") {
    GIVEN("a logger") {
        Logger logger;

        THEN("it should initialise an spdlog") {
            REQUIRE(spdlog::get("console"));
        }
    }
}

SCENARIO("reporting errors") {
    GIVEN("a reporter") {
        Reporter reporter;

        THEN("it should initialise an spdlog") {
            REQUIRE(spdlog::get("errors"));
        }

        WHEN("it is given an error to report on") {
            reporter.report(CompilerError(SourceLocation("filename")));

            THEN("it should mark the reporter as failed") {
                REQUIRE(reporter.has_errors());
            }
        }
    }
}
