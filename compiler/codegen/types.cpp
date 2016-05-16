//
// Created by Thomas Leese on 27/04/2016.
//

#include <iostream>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

#include "../Errors.h"

#include "types.h"

using namespace acorn;
using namespace acorn::codegen;

llvm::Type *TypeGenerator::take_type(ast::Node *node) {
    if (m_type_stack.size() >= 1) {
        llvm::Type *result = m_type_stack.back();
        m_type_stack.pop_back();

        if (node && result == nullptr) {
            push_error(new errors::InternalError(node, "Invalid LLVM type generated."));
            return nullptr;
        }

        return result;
    } else {
        push_error(new errors::InternalError(node, "No LLVM type generated."));
        return nullptr;
    }
}

llvm::Constant *TypeGenerator::take_initialiser(ast::Node *node) {
    if (m_initialiser_stack.size() >= 1) {
        llvm::Constant *result = m_initialiser_stack.back();
        m_initialiser_stack.pop_back();

        if (node && result == nullptr) {
            push_error(new errors::InternalError(node, "Invalid LLVM initialiser generated."));
            return nullptr;
        }

        return result;
    } else {
        push_error(new errors::InternalError(node, "No LLVM initialiser generated."));
        return nullptr;
    }
}

void TypeGenerator::push_type_parameter(types::ParameterConstructor *key, types::Type *value) {
    m_type_parameters[key] = value;
}

void TypeGenerator::pop_type_parameter(types::ParameterConstructor *key) {
    m_type_parameters.erase(key);
}

types::Type *TypeGenerator::get_type_parameter(types::ParameterConstructor *key) {
    return m_type_parameters[key];
}

types::Type *TypeGenerator::get_type_parameter(types::Parameter *key) {
    return get_type_parameter(key->constructor());
}

void TypeGenerator::visit_constructor(types::Constructor *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    llvm::Type *llvm_type = llvm::Type::getInt1Ty(context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(types::ParameterConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::AnyConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::VoidConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::BooleanConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::IntegerConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::UnsignedIntegerConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::FloatConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::UnsafePointerConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::FunctionConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::RecordConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::UnionConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::AliasConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::TypeDescriptionConstructor *type) {
    visit_constructor(type);
}

void TypeGenerator::visit(types::Parameter *type) {
    auto it = m_type_parameters.find(type->constructor());
    if (it == m_type_parameters.end()) {
        m_type_stack.push_back(nullptr);
        m_initialiser_stack.push_back(nullptr);
    } else {
        it->second->accept(this);
    }
}

void TypeGenerator::visit(types::Any *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(types::Void *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    /*llvm::Type *llvm_type = llvm::Type::getVoidTy(context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::UndefValue::get(llvm_type));*/

    llvm::Type *llvm_type = llvm::Type::getInt1Ty(context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(types::Boolean *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    llvm::Type *llvm_type = llvm::Type::getInt1Ty(context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(types::Integer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    const unsigned int size = type->size();
    llvm::Type *llvm_type = llvm::IntegerType::getIntNTy(context, size);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(types::UnsignedInteger *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    const unsigned int size = type->size();
    llvm::Type *llvm_type = llvm::IntegerType::getIntNTy(context, size);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(types::Float *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    const unsigned int size = type->size();
    llvm::Type *llvm_type = nullptr;

    switch (size) {
        case 64:
            llvm_type = llvm::Type::getDoubleTy(context);
            break;
        case 32:
            llvm_type = llvm::Type::getFloatTy(context);
            break;
        case 16:
            llvm_type = llvm::Type::getHalfTy(context);
            break;
        default:
            break;
    }

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantFP::get(llvm_type, 0));
}

void TypeGenerator::visit(types::UnsafePointer *type) {
    type->element_type()->accept(this);
    llvm::Type *element_type = take_type(nullptr);

    if (element_type) {
        llvm::PointerType *pointer_type = llvm::PointerType::getUnqual(element_type);
        m_type_stack.push_back(pointer_type);
        m_initialiser_stack.push_back(llvm::ConstantPointerNull::get(pointer_type));
    } else {
        m_type_stack.push_back(nullptr);
        m_initialiser_stack.push_back(nullptr);
    }
}

void TypeGenerator::visit(types::Record *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (auto field_type : type->field_types()) {
        field_type->accept(this);

        llvm::Type *llvm_field_type = take_type(nullptr);
        llvm::Constant *llvm_field_initialiser = take_initialiser(nullptr);

        if (llvm_field_type == nullptr || llvm_field_initialiser == nullptr) {
            m_type_stack.push_back(nullptr);
            m_initialiser_stack.push_back(nullptr);
            return;
        }

        llvm_types.push_back(llvm_field_type);
        llvm_initialisers.push_back(llvm_field_initialiser);
    }

    auto struct_type = llvm::StructType::get(context, llvm_types);
    auto struct_initialiser = llvm::ConstantStruct::get(struct_type, llvm_initialisers);

    m_type_stack.push_back(struct_type);
    m_initialiser_stack.push_back(struct_initialiser);
}

void TypeGenerator::visit(types::Tuple *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(types::Method *type) {
    type->return_type()->accept(this);
    llvm::Type *llvm_return_type = take_type(nullptr);

    if (!llvm_return_type) {
        m_type_stack.push_back(nullptr);
        m_initialiser_stack.push_back(nullptr);
        return;
    }

    std::vector<llvm::Type *> llvm_parameter_types;
    for (auto parameter_type : type->parameter_types()) {
        parameter_type->accept(this);
        auto llvm_parameter_type = take_type(nullptr);

        if (!llvm_parameter_type) {
            m_type_stack.push_back(nullptr);
            m_initialiser_stack.push_back(nullptr);
            return;
        }

        if (type->is_parameter_inout(parameter_type)) {
            llvm_parameter_type = llvm::PointerType::getUnqual(llvm_parameter_type);
        }

        llvm_parameter_types.push_back(llvm_parameter_type);
    }

    auto llvm_type = llvm::FunctionType::get(llvm_return_type,
                                             llvm_parameter_types,
                                             false);
    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(types::Function *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(types::Union *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}
