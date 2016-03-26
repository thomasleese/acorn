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
        /*compiler.compile("stdlib/builtins.jet");
        compiler.compile("stdlib/graphics.jet");
        compiler.compile("stdlib/maths.jet");*/

        /*compiler.compile("examples/types.jet");
        compiler.compile("examples/strings.jet");*/

        compiler.compile("test.jet");
    } catch (Errors::CompilerError e) {
        e.print();
        return 1;
    }

    return 0;
}
