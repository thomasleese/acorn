#include <cstdio>

#include "catch.hpp"

#include <spdlog/spdlog.h>

#include "acorn/compiler.h"

static auto logger = spdlog::stdout_color_mt("acorn");

bool compile_and_run(const std::string filename) {
    std::string full_filename = "test/examples/" + filename + ".acorn";

    acorn::compiler::Compiler compiler;
    if (compiler.parse_and_compile(full_filename) != 0) {
        return false;
    }

    std::string exe_filename = "test/examples/" + filename;

    int exit_code = system(exe_filename.c_str());

    std::remove(exe_filename.c_str());

    return exit_code == 0;
}

SCENARIO("example programs") {
    GIVEN("a program which should compile") {
        REQUIRE(compile_and_run("basic_generics"));
        REQUIRE(compile_and_run("basic_operator"));
        REQUIRE(compile_and_run("generics_1"));
        REQUIRE(compile_and_run("generics_2"));
        REQUIRE(compile_and_run("int_variable"));
        REQUIRE(compile_and_run("json"));
        REQUIRE(compile_and_run("minimal"));
        REQUIRE(compile_and_run("multiple_methods"));
        REQUIRE(compile_and_run("records"));
        REQUIRE(compile_and_run("single_method"));
    }
}
