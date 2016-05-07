#include "compiler.h"
#include "errors.h"

using namespace jet;

int main() {
    Compiler compiler;

    try {
        compiler.compile("test2.jet");
    } catch (errors::CompilerError e) {
        e.print();
        return 1;
    }

    return 0;
}
