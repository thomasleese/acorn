//
// Created by Thomas Leese on 27/04/2016.
//

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

#include "../Errors.h"

#include "types.h"

using namespace jet::codegen;

llvm::Type *TypeGenerator::take_type(AST::Node *node) {
    if (m_type_stack.size() >= 1) {
        llvm::Type *result = m_type_stack.back();
        m_type_stack.pop_back();

        if (node && result == nullptr) {
            throw Errors::InternalError(node, "Invalid LLVM type generated.");
        }

        return result;
    } else {
        if (node) {
            throw Errors::InternalError(node, "No LLVM type generated.");
        } else {
            return nullptr;
        }
    }
}

llvm::Constant *TypeGenerator::take_initialiser(AST::Node *node) {
    if (m_initialiser_stack.size() >= 1) {
        llvm::Constant *result = m_initialiser_stack.back();
        m_initialiser_stack.pop_back();

        if (node && result == nullptr) {
            throw Errors::InternalError(node, "Invalid LLVM initialiser generated.");
        }

        return result;
    } else {
        if (node) {
            throw Errors::InternalError(node, "No LLVM initialiser generated.");
        } else {
            return nullptr;
        }
    }
}

void TypeGenerator::visit(Types::AnyConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::VoidConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::BooleanConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::IntegerConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::UnsignedIntegerConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::FloatConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::UnsafePointerConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::FunctionConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::RecordConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::UnionConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::AliasConstructor *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::Any *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::Void *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    llvm::Type *llvm_type = llvm::Type::getVoidTy(context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::UndefValue::get(llvm_type));
}

void TypeGenerator::visit(Types::Boolean *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    llvm::Type *llvm_type = llvm::Type::getInt1Ty(context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(Types::Integer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    const unsigned int size = type->size();
    llvm::Type *llvm_type = llvm::IntegerType::getIntNTy(context, size);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(Types::UnsignedInteger *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    const unsigned int size = type->size();
    llvm::Type *llvm_type = llvm::IntegerType::getIntNTy(context, size);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void TypeGenerator::visit(Types::Float *type) {
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

void TypeGenerator::visit(Types::UnsafePointer *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

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

void TypeGenerator::visit(Types::Record *type) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (auto field_type : type->field_types()) {
        field_type->accept(this);

        llvm::Type *llvm_field_type = take_type(nullptr);
        llvm::Constant *llvm_field_initialiser = take_initialiser(nullptr);
        if (!llvm_field_type || !llvm_field_initialiser) {
            m_type_stack.push_back(nullptr);
            m_initialiser_stack.push_back(nullptr);
            return;
        }

        llvm_types.push_back(llvm_field_type);
        llvm_initialisers.push_back(llvm_field_initialiser);
    }

    llvm::StructType *struct_type = llvm::StructType::get(context, llvm_types);

    m_type_stack.push_back(struct_type);
    m_initialiser_stack.push_back(llvm::ConstantStruct::get(struct_type));
}

void TypeGenerator::visit(Types::Tuple *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::Method *type) {
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
        llvm::Type *llvm_parameter_type = take_type(nullptr);

        if (!llvm_parameter_type) {
            m_type_stack.push_back(nullptr);
            m_initialiser_stack.push_back(nullptr);
            return;
        }

        llvm_parameter_types.push_back(llvm_parameter_type);
    }

    auto llvm_type = llvm::FunctionType::get(llvm_return_type,
                                             llvm_parameter_types,
                                             false);
    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::Function *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}

void TypeGenerator::visit(Types::Union *type) {
    m_type_stack.push_back(nullptr);
    m_initialiser_stack.push_back(nullptr);
}
