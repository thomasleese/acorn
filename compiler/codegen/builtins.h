//
// Created by Thomas Leese on 22/05/2016.
//

#ifndef ACORN_CODEGEN_BUILTINS_H
#define ACORN_CODEGEN_BUILTINS_H

#include <string>

#include <llvm/IR/IRBuilder.h>

namespace llvm {
    class Module;
    class Function;
}

namespace acorn {

    namespace symboltable {
        struct Symbol;
    }

    namespace codegen {

        class TypeGenerator;

        class BuiltinGenerator {
        public:
            BuiltinGenerator(llvm::Module *module, llvm::IRBuilder<> *ir_builder, llvm::DataLayout *data_layout, TypeGenerator *type_generator);

            void generate(symboltable::Namespace *table);

            llvm::Function *generate_function(symboltable::Symbol *function_symbol, symboltable::Symbol *method_symbol, std::string llvm_name);

        private:
            void generate_sizeof(types::Method *method, llvm::Function *function);
            void generate_strideof(types::Method *method, llvm::Function *function);

            llvm::Function *create_llvm_function(symboltable::Namespace *table, std::string name, int index);

            void initialise_boolean_variable(symboltable::Namespace *table, std::string name, bool value);
            void initialise_function(llvm::Function *function, int no_arguments);

        private:
            llvm::Module *m_module;
            llvm::IRBuilder<> *m_ir_builder;
            llvm::DataLayout *m_data_layout;
            TypeGenerator *m_type_generator;

            std::vector<llvm::Argument *> m_args;
        };

    }

}

#endif // ACORN_CODEGEN_BUILTINS_H
