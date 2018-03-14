#include <iostream>
#include <memory>

#include <llvm/Support/CommandLine.h>

#include "acorn/compiler.h"
#include "acorn/prettyprinter.h"

using namespace acorn;

llvm::cl::opt<std::string> input_filename(
    llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::Required
);

int main(int argc, char *argv[]) {
    llvm::cl::ParseCommandLineOptions(argc, argv);

    compiler::Compiler compiler;
    return compiler.parse_and_compile(input_filename);
}
