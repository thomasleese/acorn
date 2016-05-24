//
// Created by Thomas Leese on 22/05/2016.
//

#include "../symboltable.h"
#include "module.h"
#include "types.h"

#include "builtins.h"

using namespace acorn;
using namespace acorn::codegen;

BuiltinGenerator::BuiltinGenerator(llvm::Module *module, llvm::IRBuilder<> *ir_builder, llvm::DataLayout *data_layout, TypeGenerator *type_generator) :
        m_module(module),
        m_ir_builder(ir_builder),
        m_data_layout(data_layout),
        m_type_generator(type_generator)
{

}

void BuiltinGenerator::generate(symboltable::Namespace *table) {
    initialise_boolean_variable(table, "Nothing", false);
    initialise_boolean_variable(table, "True", true);
    initialise_boolean_variable(table, "False", false);
    initialise_boolean_variable(table, "Integer32", false);
    initialise_boolean_variable(table, "Integer64", false);
    initialise_boolean_variable(table, "UnsafePointer", false);

    // not
    llvm::Function *f = create_llvm_function(table, "not", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateNot(m_args[0]));

    // multiplication
    f = create_llvm_function(table, "*", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateMul(m_args[0], m_args[1]));

    // addition
    f = create_llvm_function(table, "+", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateAdd(m_args[0], m_args[1]));

    f = create_llvm_function(table, "+", 1);
    m_ir_builder->CreateRet(m_ir_builder->CreateAdd(m_args[0], m_args[1]));

    f = create_llvm_function(table, "+", 2);
    m_ir_builder->CreateRet(m_ir_builder->CreateFAdd(m_args[0], m_args[1]));

    // subtraction
    f = create_llvm_function(table, "-", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateSub(m_args[0], m_args[1]));

    // equality
    for (int i = 0; i < 3; i++) {
        f = create_llvm_function(table, "==", 0);
        m_ir_builder->CreateRet(m_ir_builder->CreateICmpEQ(m_args[0], m_args[1]));
    }

    // not equality
    f = create_llvm_function(table, "!=", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateICmpNE(m_args[0], m_args[1]));

    // less than
    f = create_llvm_function(table, "<", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateICmpSLT(m_args[0], m_args[1]));

    // greater than or equal to
    f = create_llvm_function(table, ">=", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateICmpSGE(m_args[0], m_args[1]));

    // to integer
    f = create_llvm_function(table, "to_integer", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateFPToSI(m_args[0], f->getReturnType()));

    // to integer
    f = create_llvm_function(table, "to_float", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateSIToFP(m_args[0], f->getReturnType()));
}

llvm::Function *BuiltinGenerator::generate_function(symboltable::Symbol *function_symbol, symboltable::Symbol *method_symbol, std::string llvm_name) {
    auto method = dynamic_cast<types::Method *>(method_symbol->type);
    method->accept(m_type_generator);

    auto type = static_cast<llvm::FunctionType *>(m_type_generator->take_type(nullptr));
    assert(type);

    auto old_insert_point = m_ir_builder->saveIP();

    auto function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_name, m_module);
    initialise_function(function, method->parameter_types().size());

    if (function_symbol->name == "sizeof") {
        generate_sizeof(method, function);
    } else if (function_symbol->name == "strideof") {
        generate_strideof(method, function);
    } else {
        std::vector<llvm::Value *> index_list;
        index_list.push_back(m_args[1]);
        auto gep = m_ir_builder->CreateInBoundsGEP(m_args[0], index_list);

        if (function_symbol->name == "setindex") {
            m_ir_builder->CreateStore(m_args[2], gep);
            m_ir_builder->CreateRet(m_ir_builder->getInt1(false));
        } else {
            m_ir_builder->CreateRet(m_ir_builder->CreateLoad(gep));
        }
    }

    m_ir_builder->restoreIP(old_insert_point);

    return function;
}

void BuiltinGenerator::generate_sizeof(types::Method *method, llvm::Function *function) {
    auto type = dynamic_cast<types::TypeType *>(method->parameter_types()[0]);
    type->create(nullptr, nullptr)->accept(m_type_generator);
    auto llvm_type = m_type_generator->take_type(nullptr);

    uint64_t size = m_data_layout->getTypeStoreSize(llvm_type);
    m_ir_builder->CreateRet(m_ir_builder->getInt64(size));
}

void BuiltinGenerator::generate_strideof(types::Method *method, llvm::Function *function) {
    auto type = dynamic_cast<types::TypeType *>(method->parameter_types()[0]);
    type->create(nullptr, nullptr)->accept(m_type_generator);
    auto llvm_type = m_type_generator->take_type(nullptr);

    uint64_t size = m_data_layout->getTypeAllocSize(llvm_type);
    m_ir_builder->CreateRet(m_ir_builder->getInt64(size));
}

llvm::Function *BuiltinGenerator::create_llvm_function(symboltable::Namespace *table, std::string name, int index) {
    types::Function *functionType = static_cast<types::Function *>(table->lookup(nullptr, nullptr, name)->type);
    types::Method *methodType = functionType->get_method(index);

    std::string mangled_name = codegen::mangle_method(name, methodType);

    auto type_generator = new codegen::TypeGenerator();
    methodType->accept(type_generator);

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(type_generator->take_type(nullptr));
    assert(type);

    delete type_generator;

    llvm::Function *f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, mangled_name, m_module);
    f->addFnAttr(llvm::Attribute::AlwaysInline);

    initialise_function(f, methodType->parameter_types().size());

    return f;
}

void BuiltinGenerator::initialise_boolean_variable(symboltable::Namespace *table, std::string name, bool value) {
    auto symbol = table->lookup(nullptr, nullptr, name);
    symbol->value = new llvm::GlobalVariable(*m_module, m_ir_builder->getInt1Ty(), false,
                                             llvm::GlobalValue::InternalLinkage,
                                             m_ir_builder->getInt1(value), name);
}

void BuiltinGenerator::initialise_function(llvm::Function *function, int no_arguments) {
    m_args.clear();

    if (!function->arg_empty()) {
        m_args.push_back(&function->getArgumentList().front());
        for (int i = 1; i < function->arg_size(); i++) {
            m_args.push_back(function->getArgumentList().getNext(m_args[i - 1]));
        }
    }

    assert(m_args.size() == function->arg_size());
    assert(m_args.size() == no_arguments);

    auto &context = function->getContext();

    auto basic_block = llvm::BasicBlock::Create(context, "entry", function);
    m_ir_builder->SetInsertPoint(basic_block);
}
