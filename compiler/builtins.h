//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef ACORN_BUILTINS_H
#define ACORN_BUILTINS_H

#include <map>
#include <string>

#include <llvm/IR/IRBuilder.h>

namespace llvm {
    class Module;
}

namespace acorn {

    namespace codegen {
        class TypeGenerator;
    }

    namespace symboltable {
        class Namespace;
        struct Symbol;
    }

    namespace types {
        class Type;
        class ParameterType;
        class Method;
    }

    namespace builtins {

        void fill_symbol_table(symboltable::Namespace *table);
        void fill_llvm_module(symboltable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder);

        void generate_function(symboltable::Symbol *function_symbol, symboltable::Symbol *method_symbol, types::Method *method, std::string llvm_name, llvm::Module *module, llvm::IRBuilder<> *irBuilder, codegen::TypeGenerator *type_generator);

    };
}

#endif //ACORN_BUILTINS_H
