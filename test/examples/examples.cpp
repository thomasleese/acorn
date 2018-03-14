#include <cstdio>

#include "catch.hpp"

#include "acorn/compiler.h"

bool compile_and_run(const std::string filename) {
    std::string full_filename = "test/examples/" + filename + ".acorn";

    std::string exe_filename = "test/examples/" + filename;

    acorn::compiler::Compiler compiler;
    if (compiler.parse_and_compile(full_filename) != 0) {
        std::remove(exe_filename.c_str());
        return false;
    }

    int exit_code = system(exe_filename.c_str());

    std::remove(exe_filename.c_str());

    return exit_code == 0;
}

SCENARIO("example programs") {
    GIVEN("a program which should compile") {
        REQUIRE(compile_and_run("generics"));
        REQUIRE(compile_and_run("minimal"));
        REQUIRE(compile_and_run("records"));
    }
}
