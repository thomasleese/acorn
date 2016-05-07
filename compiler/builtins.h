//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_BUILTINS_H
#define JET_BUILTINS_H

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

namespace llvm {
    class Module;
}

namespace SymbolTable {
    class Namespace;
}

namespace jet {

    namespace builtins {

        void fill_symbol_table(SymbolTable::Namespace *table);
        void fill_llvm_module(SymbolTable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder);

    };
}

#endif //JET_BUILTINS_H
