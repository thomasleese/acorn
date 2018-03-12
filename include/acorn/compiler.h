//
// Created by Thomas Leese on 14/03/2016.
//

#pragma once

#include <string>
#include <vector>

#include <llvm/IR/LLVMContext.h>

#include "diagnostics.h"

namespace llvm {
    class TargetMachine;
}

namespace acorn {

    namespace ast {
        class SourceFile;
    }

    namespace symboltable {
        class Namespace;
    }

}

namespace acorn::compiler {

    class Compiler : public diagnostics::Reporter {

    public:
        Compiler();
        ~Compiler();

        bool compile(ast::SourceFile *module, symboltable::Namespace *root_namespace, std::string filename);

    private:
        llvm::Triple get_triple() const;
        llvm::TargetMachine *get_target_machine(llvm::Triple triple) const;

    private:
        llvm::LLVMContext m_context;

    };
}
