#include <iostream>

#include <llvm/Support/TargetSelect.h>

#include "Compiler.h"
#include "Errors.h"

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    Compiler compiler;

    try {
        compiler.compile("test.jet");
    } catch (Errors::CompilerError e) {
        e.print();
        return 1;
    }

    return 0;
}
