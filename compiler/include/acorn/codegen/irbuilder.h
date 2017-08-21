//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

namespace llvm {
    class Module;
    class Function;
}

namespace acorn::codegen {

    class IrBuilder {
    public:
        explicit IrBuilder(llvm::LLVMContext &context);

        void push_insert_point();
        void pop_insert_point();
        llvm::IRBuilderBase::InsertPoint insert_point() const;

        llvm::BasicBlock *create_basic_block(std::string name, llvm::Function *function = nullptr, bool set_insert_point = false);
        llvm::BasicBlock *create_entry_basic_block(llvm::Function *function = nullptr, bool set_insert_point = false);

        std::vector<llvm::Value *> build_gep_index(std::initializer_list<int> indexes);
        llvm::Value *create_inbounds_gep(llvm::Value *value, std::initializer_list<int> indexes);
        llvm::Value *create_store_method_to_function(llvm::Function *method, llvm::Value *function, int method_index, int specialisation_index);

    protected:
        llvm::IRBuilder<> *m_ir_builder;

    private:
        std::vector<llvm::IRBuilderBase::InsertPoint> m_insert_points;
    };

}
