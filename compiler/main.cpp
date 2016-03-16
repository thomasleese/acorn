#include <iostream>

#include "Compiler.h"
#include "Errors.h"

int main() {
    Compiler compiler;

    try {
        compiler.compile("stdlib/builtins.jet");
        compiler.compile("stdlib/graphics.jet");
        compiler.compile("stdlib/maths.jet");
        compiler.compile("example.jet");
    } catch (Errors::CompilerError e) {
        e.print();
        return 1;
    }

    return 0;
}
