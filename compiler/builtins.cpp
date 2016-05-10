//
// Created by Thomas Leese on 18/03/2016.
//

#include <iostream>
#include <sstream>

#include <llvm/IR/Module.h>

#include "symboltable.h"
#include "types.h"
#include "codegen/module.h"
#include "codegen/types.h"

#include "builtins.h"

using namespace jet;
using namespace jet::builtins;

symboltable::Symbol *add_symbol(symboltable::Namespace *table, std::string name, types::Type *type) {
    symboltable::Symbol *symbol = new symboltable::Symbol(name);
    symbol->type = type;
    symbol->is_builtin = true;
    table->insert(nullptr, nullptr, symbol);
    return symbol;
}

symboltable::Symbol *add_base_function(symboltable::Namespace *table, std::string name) {
    symboltable::Symbol *symbol = new symboltable::Symbol(name);
    types::Function *type = new types::Function();
    symbol->type = type;
    symbol->nameSpace = new symboltable::Namespace(table);
    table->insert(nullptr, nullptr, symbol);
    return symbol;
}

void add_base_method(symboltable::Symbol *function, types::Method *method) {
    symboltable::Symbol *symbol = new symboltable::Symbol(method->mangled_name());
    symbol->type = method;
    symbol->nameSpace = new symboltable::Namespace(function->nameSpace);
    function->nameSpace->insert(nullptr, nullptr, symbol);

    auto function_type = static_cast<types::Function *>(function->type);
    function_type->add_method(method);
}

void add_base_type_constructors(symboltable::Namespace *table) {
    add_symbol(table, "Any", new types::AnyConstructor());
    add_symbol(table, "Void", new types::VoidConstructor());
    add_symbol(table, "Boolean", new types::BooleanConstructor());
    add_symbol(table, "Integer8", new types::IntegerConstructor(8));
    add_symbol(table, "Integer16", new types::IntegerConstructor(16));
    add_symbol(table, "Integer32", new types::IntegerConstructor(32));
    add_symbol(table, "Integer64", new types::IntegerConstructor(64));
    add_symbol(table, "Integer128", new types::IntegerConstructor(128));
    add_symbol(table, "UnsignedInteger8", new types::UnsignedIntegerConstructor(8));
    add_symbol(table, "UnsignedInteger16", new types::UnsignedIntegerConstructor(16));
    add_symbol(table, "UnsignedInteger32", new types::UnsignedIntegerConstructor(32));
    add_symbol(table, "UnsignedInteger64", new types::UnsignedIntegerConstructor(64));
    add_symbol(table, "UnsignedInteger128", new types::UnsignedIntegerConstructor(128));
    add_symbol(table, "Float16", new types::FloatConstructor(16));
    add_symbol(table, "Float32", new types::FloatConstructor(32));
    add_symbol(table, "Float64", new types::FloatConstructor(64));
    add_symbol(table, "Float128", new types::FloatConstructor(128));
    add_symbol(table, "UnsafePointer", new types::UnsafePointerConstructor());
    add_symbol(table, "Function", new types::FunctionConstructor());
    add_symbol(table, "Union", new types::UnionConstructor());
}

void builtins::fill_symbol_table(symboltable::Namespace *table) {
    add_base_type_constructors(table);

    add_symbol(table, "Nothing", new types::Void());

    symboltable::Symbol *multiplication = add_base_function(table, "*");
    add_base_method(multiplication, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    symboltable::Symbol *addition = add_base_function(table, "+");
    add_base_method(addition, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));
    add_base_method(addition, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::UnsignedInteger(64)));
    add_base_method(addition, new types::Method(new types::Float(64), new types::Float(64), new types::Float(64)));

    symboltable::Symbol *subtraction = add_base_function(table, "-");
    add_base_method(subtraction, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    symboltable::Symbol *equality = add_base_function(table, "==");
    add_base_method(equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));
    add_base_method(equality, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::Boolean()));

    symboltable::Symbol *not_equality = add_base_function(table, "!=");
    add_base_method(not_equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    symboltable::Symbol *less_than = add_base_function(table, "<");
    add_base_method(less_than, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    symboltable::Symbol *to_integer = add_base_function(table, "to_integer");
    add_base_method(to_integer, new types::Method(new types::Float(64), new types::Integer(64)));

    symboltable::Symbol *to_float = add_base_function(table, "to_float");
    add_base_method(to_float, new types::Method(new types::Integer(64), new types::Float(64)));
}

llvm::Function *create_llvm_function(symboltable::Namespace *table, llvm::Module *module, std::string name, int index) {
    types::Function *functionType = static_cast<types::Function *>(table->lookup(nullptr, nullptr, name)->type);
    types::Method *methodType = functionType->get_method(index);

    std::string mangled_name = codegen::mangle_method(name, methodType);

    auto type_generator = new codegen::TypeGenerator();
    methodType->accept(type_generator);

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(type_generator->take_type(nullptr));
    assert(type);

    delete type_generator;

    llvm::Function *f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, mangled_name, module);
    f->addFnAttr(llvm::Attribute::AlwaysInline);

    return f;
}

static llvm::Argument *self;
static llvm::Argument *lhs;
static llvm::Argument *rhs;

void initialise_function_block(llvm::Function *function, llvm::IRBuilder<> *irBuilder) {
    llvm::LLVMContext &context = function->getContext();

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(context, "entry", function);
    irBuilder->SetInsertPoint(basicBlock);
}

void initialise_binary_function(llvm::Function *function, llvm::IRBuilder<> *irBuilder) {
    lhs = &function->getArgumentList().front();
    rhs = &function->getArgumentList().back();

    initialise_function_block(function, irBuilder);
}

void initialise_unary_function(llvm::Function *function, llvm::IRBuilder<> *irBuilder) {
    self = &function->getArgumentList().front();

    initialise_function_block(function, irBuilder);
}

void builtins::fill_llvm_module(symboltable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder) {
    // multiplication
    llvm::Function *f = create_llvm_function(table, module, "*", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateMul(lhs, rhs));

    // addition
    f = create_llvm_function(table, module, "+", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateAdd(lhs, rhs));

    f = create_llvm_function(table, module, "+", 1);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateAdd(lhs, rhs));

    f = create_llvm_function(table, module, "+", 2);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateFAdd(lhs, rhs));

    // subtraction
    f = create_llvm_function(table, module, "-", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateSub(lhs, rhs));

    // equality
    f = create_llvm_function(table, module, "==", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpEQ(lhs, rhs));

    f = create_llvm_function(table, module, "==", 1);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpEQ(lhs, rhs));

    // not equality
    f = create_llvm_function(table, module, "!=", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpNE(lhs, rhs));

    // less than
    f = create_llvm_function(table, module, "<", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpSLT(lhs, rhs));

    // to integer
    f = create_llvm_function(table, module, "to_integer", 0);
    initialise_unary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateFPToSI(self, f->getReturnType()));

    // to integer
    f = create_llvm_function(table, module, "to_float", 0);
    initialise_unary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateSIToFP(self, f->getReturnType()));
}
