#include <iostream>

#include "Compiler.h"

using namespace std;

int main() {
    Compiler compiler;
    compiler.compile("example.quark");

    return 0;
}
