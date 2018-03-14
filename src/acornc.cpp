#include <iostream>
#include <memory>

#include <llvm/Support/CommandLine.h>

#include "acorn/compiler.h"
#include "acorn/prettyprinter.h"

using namespace acorn;

void pretty_print(ast::SourceFile *source_file) {
    PrettyPrinter pp;
    pp.visit_source_file(source_file);
    pp.print();
}

llvm::cl::opt<std::string> input_filename(
    llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::Required
);

int main(int argc, char *argv[]) {
    llvm::cl::ParseCommandLineOptions(argc, argv);

    compiler::Compiler compiler;
    return compiler.parse_and_compile(input_filename);
}
