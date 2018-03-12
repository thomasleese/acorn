#include <iostream>
#include <memory>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/compiler.h"
#include "acorn/prettyprinter.h"
#include "acorn/symboltable/namespace.h"

using namespace acorn;

static auto logger = spdlog::stdout_color_mt("acorn");

void pretty_print(ast::SourceFile *source_file) {
    PrettyPrinter pp;
    pp.visit_source_file(source_file);
    pp.print();
}

int main(int argc, char *argv[]) {
    logger->set_level(spdlog::level::debug);
    logger->set_pattern("[%H:%M:%S] %v");

    std::string filename = argv[1];

    compiler::Compiler compiler;
    return compiler.parse_and_compile(filename);
}
