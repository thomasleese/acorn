//
// Created by Thomas Leese on 27/04/2016.
//

#include <llvm/IR/Module.h>

#include "../Errors.h"

#include "types.h"

using namespace jet::codegen;

llvm::Type *TypeGenerator::take_result(AST::Node *node) {
    if (m_stack.size() == 1) {
        llvm::Type *result = m_stack.back();
        m_stack.pop_back();
        return result;
    } else {
        throw Errors::InternalError(node, "No LLVM type generated.");
    }
}

void TypeGenerator::visit(Types::Constructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::AnyConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::VoidConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::BooleanConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::IntegerConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::UnsignedIntegerConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::FloatConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::UnsafePointerConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::FunctionConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::RecordConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::UnionConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::AliasConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Any *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Void *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Boolean *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Integer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::UnsignedInteger *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Float *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::UnsafePointer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Record *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Tuple *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Method *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Function *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

void TypeGenerator::visit(Types::Union *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_type(context));
}

llvm::Constant *TypeInitialiserGenerator::take_result(AST::Node *node) {
    if (m_stack.size() == 1) {
        llvm::Constant *result = m_stack.back();
        m_stack.pop_back();
        return result;
    } else {
        throw Errors::InternalError(node, "No LLVM variable initialiser generated.");
    }
}

void TypeInitialiserGenerator::visit(Types::Constructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::AnyConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::VoidConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::BooleanConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::IntegerConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::UnsignedIntegerConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::FloatConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::UnsafePointerConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::FunctionConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::RecordConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::UnionConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::AliasConstructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Any *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Void *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Boolean *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Integer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::UnsignedInteger *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Float *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::UnsafePointer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Record *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Tuple *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Method *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Function *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}

void TypeInitialiserGenerator::visit(Types::Union *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    m_stack.push_back(type->create_llvm_initialiser(context));
}
