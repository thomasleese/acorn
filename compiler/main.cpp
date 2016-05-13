#include "compiler/compiler.h"

using namespace jet;

int main() {
    compiler::Compiler compiler;
    if (compiler.compile("test.acorn")) {
        return 0;
    } else {
        return 1;
    }
}
