#include <iostream>
#include <memory>

#include "acorn/compiler.h"
#include "acorn/prettyprinter.h"

using namespace acorn;

void pretty_print(ast::SourceFile *source_file) {
    PrettyPrinter pp;
    pp.visit_source_file(source_file);
    pp.print();
}

int main(int argc, char *argv[]) {
    std::string filename = argv[1];

    compiler::Compiler compiler;
    return compiler.parse_and_compile(filename);
}
