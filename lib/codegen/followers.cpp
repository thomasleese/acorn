//
// Created by Thomas Leese on 12/01/2017.
//

#include <spdlog/spdlog.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Value.h>

#include "acorn/codegen/followers.h"

using namespace acorn;
using namespace acorn::codegen;

static auto logger = spdlog::get("acorn");

void ValueFollower::push_llvm_value(llvm::Value *value) {
    m_llvm_value_stack.push_back(value);
}

bool ValueFollower::has_llvm_value() const {
    return !m_llvm_value_stack.empty();
}

llvm::Value *ValueFollower::pop_llvm_value() {
    if (!has_llvm_value()) {
        logger->warn("ValueFollower::pop_llvm_value with nothing available");
        return nullptr;
    }

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
    if (!has_llvm_type()) {
        logger->warn("TypeFollower::pop_llvm_type with nothing available");
        return nullptr;
    }

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
    if (!has_llvm_initialiser()) {
        logger->warn("InitialiserFollower::pop_llvm_initialiser with nothing available");
        return nullptr;
    }

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
