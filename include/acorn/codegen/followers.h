#pragma once

#include <vector>

#include "../diagnostics.h"

namespace llvm {
    class Value;
    class Type;
    class Constant;
}

namespace acorn::codegen {

    class ValueFollower {
    public:
        void push_llvm_value(llvm::Value *value);
        llvm::Value *pop_llvm_value();
        bool has_llvm_value() const;
        llvm::Value *llvm_value() const;

    private:
        diagnostics::Logger m_logger;
        std::vector<llvm::Value *> m_llvm_value_stack;
    };

    class TypeFollower {
    public:
        void push_llvm_type(llvm::Type *type);
        llvm::Type *pop_llvm_type();
        bool has_llvm_type() const;
        llvm::Type *llvm_type() const;

    private:
        diagnostics::Logger m_logger;
        std::vector<llvm::Type *> m_llvm_type_stack;
    };

    class InitialiserFollower {
    public:
        void push_llvm_initialiser(llvm::Constant *initialiser);
        llvm::Constant *pop_llvm_initialiser();
        bool has_llvm_initialiser() const;
        llvm::Constant *llvm_initialiser() const;

    private:
        diagnostics::Logger m_logger;
        std::vector<llvm::Constant *> m_llvm_initialiser_stack;
    };

}
