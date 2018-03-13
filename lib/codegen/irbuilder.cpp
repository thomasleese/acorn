#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include "acorn/codegen/irbuilder.h"

using namespace acorn;
using namespace acorn::codegen;

IrBuilder::IrBuilder(llvm::LLVMContext &context)
    : m_ir_builder(new llvm::IRBuilder<>(context)) { }

void IrBuilder::push_insert_point() {
    m_insert_points.push_back(m_ir_builder->saveIP());
}

void IrBuilder::pop_insert_point() {
    m_ir_builder->restoreIP(insert_point());
    m_insert_points.pop_back();
}

llvm::IRBuilderBase::InsertPoint IrBuilder::insert_point() const {
    return m_insert_points.back();
}

llvm::BasicBlock *IrBuilder::create_basic_block(std::string name, llvm::Function *function, bool set_insert_point) {
    if (function == nullptr) {
        function = m_ir_builder->GetInsertBlock()->getParent();
    }

    auto bb = llvm::BasicBlock::Create(function->getContext(), name, function);

    if (set_insert_point) {
        m_ir_builder->SetInsertPoint(bb);
    }

    return bb;
}

llvm::BasicBlock *IrBuilder::create_entry_basic_block(llvm::Function *function, bool set_insert_point) {
    return create_basic_block("entry", function, set_insert_point);
}

std::vector<llvm::Value *> IrBuilder::build_gep_index(std::initializer_list<int> indexes) {
    std::vector<llvm::Value *> values;
    for (int index : indexes) {
        values.push_back(m_ir_builder->getInt32(index));
    }
    return values;
}

llvm::Value *IrBuilder::create_inbounds_gep(llvm::Value *value, std::initializer_list<int> indexes) {
    return m_ir_builder->CreateInBoundsGEP(value, build_gep_index(indexes));
}

llvm::Value *IrBuilder::create_store_method_to_function(llvm::Function *method, llvm::Value *function, int method_index, int specialisation_index) {
    auto method_holder = create_inbounds_gep(function, { 0, method_index, specialisation_index });
    return m_ir_builder->CreateStore(method, method_holder);
}
