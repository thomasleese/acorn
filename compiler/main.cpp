#include <iostream>

#include "Compiler.h"
#include "Errors.h"

int main() {
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
