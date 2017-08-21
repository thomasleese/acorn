//
// Created by Thomas Leese on 12/01/2017.
//

#include <iostream>
#include <memory>
#include <sstream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/symboltable/namespace.h"
#include "acorn/symboltable/symbol.h"
#include "acorn/typesystem/types.h"
#include "acorn/utils.h"

#include "acorn/codegen/followers.h"

using namespace acorn;
using namespace acorn::codegen;
using namespace acorn::diagnostics;

void ValueFollower::push_llvm_value(llvm::Value *value) {
    m_llvm_value_stack.push_back(value);
}

bool ValueFollower::has_llvm_value() const {
    return !m_llvm_value_stack.empty();
}

llvm::Value *ValueFollower::pop_llvm_value() {
    assert(has_llvm_value());
    auto value = m_llvm_value_stack.back();
    m_llvm_value_stack.pop_back();
    return value;
}

llvm::Value *ValueFollower::llvm_value() const {
    return m_llvm_value_stack.back();
}

void TypeFollower::push_llvm_type(llvm::Type *type) {
    m_llvm_type_stack.push_back(type);
}

llvm::Type *TypeFollower::pop_llvm_type() {
    assert(has_llvm_type());
    auto value = m_llvm_type_stack.back();
    m_llvm_type_stack.pop_back();
    return value;
}

bool TypeFollower::has_llvm_type() const {
    return !m_llvm_type_stack.empty();
}

llvm::Type *TypeFollower::llvm_type() const {
    return m_llvm_type_stack.back();
}

void InitialiserFollower::push_llvm_initialiser(llvm::Constant *initialiser) {
    m_llvm_initialiser_stack.push_back(initialiser);
}

llvm::Constant *InitialiserFollower::pop_llvm_initialiser() {
    assert(has_llvm_initialiser());
    auto value = m_llvm_initialiser_stack.back();
    m_llvm_initialiser_stack.pop_back();
    return value;
}

bool InitialiserFollower::has_llvm_initialiser() const {
    return !m_llvm_initialiser_stack.empty();
}

llvm::Constant *InitialiserFollower::llvm_initialiser() const {
    return m_llvm_initialiser_stack.back();
}
