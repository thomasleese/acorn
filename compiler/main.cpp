#include "compiler.h"

using namespace acorn;

int main() {
    compiler::Compiler compiler;
    if (compiler.compile("test.acorn")) {
        return 0;
    } else {
        return 1;
    }
}
