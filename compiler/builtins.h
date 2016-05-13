//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef ACORN_BUILTINS_H
#define ACORN_BUILTINS_H

#include <llvm/IR/IRBuilder.h>

namespace llvm {
    class Module;
}

namespace acorn {

    namespace symboltable {
        class Namespace;
    }

    namespace builtins {

        void fill_symbol_table(symboltable::Namespace *table);
        void fill_llvm_module(symboltable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder);

    };
}

#endif //ACORN_BUILTINS_H
