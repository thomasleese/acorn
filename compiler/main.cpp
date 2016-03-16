#include <iostream>

#include "Compiler.h"
#include "Errors.h"

int main() {
    Compiler compiler;

    try {
        /*compiler.compile("stdlib/builtins.quark");
        compiler.compile("stdlib/graphics.quark");
        compiler.compile("stdlib/maths.quark");*/
        compiler.compile("example.jet");
    } catch (Errors::CompilerError e) {
        e.print();
        return 1;
    }

    return 0;
}
