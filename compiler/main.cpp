#include <iostream>

#include "Compiler.h"
#include "Errors.h"

int main() {
    Compiler compiler;

    try {
        compiler.compile("test2.jet");
    } catch (Errors::CompilerError e) {
        e.print();
        return 1;
    }

    return 0;
}
