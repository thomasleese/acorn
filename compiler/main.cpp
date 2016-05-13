#include "compiler/compiler.h"
#include "errors.h"

using namespace jet;

int main() {
    compiler::Compiler compiler;
    if (compiler.compile("test.jet")) {
        return 0;
    } else {
        return 1;
    }
}
