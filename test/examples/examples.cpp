#include <cstdio>

#include "catch.hpp"

#include "acorn/compiler.h"

int compile_and_run(const std::string filename) {
    std::string full_filename = "test/examples/" + filename + ".acorn";

    std::string exe_filename = "test/examples/" + filename;

    acorn::compiler::Compiler compiler;
    if (compiler.parse_and_compile(full_filename) != 0) {
        std::remove(exe_filename.c_str());
        return -1;
    }

    int exit_code = system(exe_filename.c_str());

    std::remove(exe_filename.c_str());

    return exit_code;
}

SCENARIO("example programs") {
    GIVEN("a program which should compile") {
        REQUIRE(compile_and_run("generics") == 0);
        REQUIRE(compile_and_run("minimal") == 0);
        REQUIRE(compile_and_run("records") == 0);
    }
}
