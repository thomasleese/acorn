#include "compiler/compiler.h"
#include "errors.h"

using namespace jet;

int main() {
    compiler::Compiler compiler;
    if (compiler.compile("test2.jet")) {
        return 0;
    } else {
        return 1;
    }
}
