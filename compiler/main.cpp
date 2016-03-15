#include <iostream>

#include "Compiler.h"

using namespace std;

int main() {
    Compiler compiler;
    compiler.compile("stdlib/builtins.quark");
    compiler.compile("stdlib/graphics.quark");
    compiler.compile("stdlib/maths.quark");
    compiler.compile("example.quark");

    return 0;
}
