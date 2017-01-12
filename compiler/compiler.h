//
// Created by Thomas Leese on 14/03/2016.
//

#pragma once

#include <string>
#include <vector>

#include <llvm/IR/LLVMContext.h>

namespace acorn {

    namespace compiler {

        class Compiler {

        public:
            Compiler();
            ~Compiler();

            void debug(std::string line) const;
            bool compile(std::string filename);

        private:
            llvm::LLVMContext m_context;

        };
    }

}
