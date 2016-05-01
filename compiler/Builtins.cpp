//
// Created by Thomas Leese on 18/03/2016.
//

#include <iostream>
#include <sstream>

#include "Mangler.h"
#include "SymbolTable.h"
#include "Types.h"
#include "codegen/types.h"

#include "Builtins.h"

using namespace jet;

SymbolTable::Symbol *add_symbol(SymbolTable::Namespace *table, std::string name, Types::Type *type) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    symbol->type = type;
    symbol->is_builtin = true;
    table->insert(nullptr, symbol);
    return symbol;
}

SymbolTable::Symbol *add_base_function(SymbolTable::Namespace *table, std::string name) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    Types::Function *type = new Types::Function();
    symbol->type = type;
    symbol->nameSpace = new SymbolTable::Namespace(table);
    table->insert(nullptr, symbol);
    return symbol;
}

void add_base_method(SymbolTable::Symbol *function, Types::Method *method) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(method->mangled_name());
    symbol->type = method;
    symbol->nameSpace = new SymbolTable::Namespace(function->nameSpace);
    function->nameSpace->insert(nullptr, symbol);

    Types::Function *f = static_cast<Types::Function *>(function->type);
    f->add_method(method);
}

void add_base_type_constructors(SymbolTable::Namespace *table) {
    add_symbol(table, "Any", new Types::AnyConstructor());
    add_symbol(table, "Void", new Types::VoidConstructor());
    add_symbol(table, "Boolean", new Types::BooleanConstructor());
    add_symbol(table, "Integer8", new Types::IntegerConstructor(8));
    add_symbol(table, "Integer16", new Types::IntegerConstructor(16));
    add_symbol(table, "Integer32", new Types::IntegerConstructor(32));
    add_symbol(table, "Integer64", new Types::IntegerConstructor(64));
    add_symbol(table, "Integer128", new Types::IntegerConstructor(128));
    add_symbol(table, "UnsignedInteger8", new Types::UnsignedIntegerConstructor(8));
    add_symbol(table, "UnsignedInteger16", new Types::UnsignedIntegerConstructor(16));
    add_symbol(table, "UnsignedInteger32", new Types::UnsignedIntegerConstructor(32));
    add_symbol(table, "UnsignedInteger64", new Types::UnsignedIntegerConstructor(64));
    add_symbol(table, "UnsignedInteger128", new Types::UnsignedIntegerConstructor(128));
    add_symbol(table, "Float16", new Types::FloatConstructor(16));
    add_symbol(table, "Float32", new Types::FloatConstructor(32));
    add_symbol(table, "Float64", new Types::FloatConstructor(64));
    add_symbol(table, "Float128", new Types::FloatConstructor(128));
    add_symbol(table, "UnsafePointer", new Types::UnsafePointerConstructor());
    add_symbol(table, "Function", new Types::FunctionConstructor());
    add_symbol(table, "Union", new Types::UnionConstructor());
}

void Builtins::fill_symbol_table(SymbolTable::Namespace *table) {
    add_base_type_constructors(table);

    add_symbol(table, "Nothing", new Types::Void());

    SymbolTable::Symbol *multiplication = add_base_function(table, "*");
    add_base_method(multiplication, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));

    SymbolTable::Symbol *addition = add_base_function(table, "+");
    add_base_method(addition, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));
    add_base_method(addition, new Types::Method("a", new Types::Float(64), "b", new Types::Float(64), new Types::Float(64)));

    SymbolTable::Symbol *subtraction = add_base_function(table, "-");
    add_base_method(subtraction, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));

    SymbolTable::Symbol *equality = add_base_function(table, "==");
    add_base_method(equality, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Boolean()));

    SymbolTable::Symbol *not_equality = add_base_function(table, "!=");
    add_base_method(not_equality, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Boolean()));

    SymbolTable::Symbol *less_than = add_base_function(table, "<");
    add_base_method(less_than, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Boolean()));

    SymbolTable::Symbol *to_integer = add_base_function(table, "to_integer");
    add_base_method(to_integer, new Types::Method("self", new Types::Float(64), new Types::Integer(64)));

    SymbolTable::Symbol *to_float = add_base_function(table, "to_float");
    add_base_method(to_float, new Types::Method("self", new Types::Integer(64), new Types::Float(64)));
}

llvm::Function *create_llvm_function(SymbolTable::Namespace *table, llvm::Module *module, std::string name, int index) {
    Types::Function *functionType = static_cast<Types::Function *>(table->lookup(nullptr, name)->type);
    Types::Method *methodType = functionType->get_method(index);

    std::string mangled_name = Mangler::mangle_method(name, methodType);

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

void Builtins::fill_llvm_module(SymbolTable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder) {
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
    irBuilder->CreateRet(irBuilder->CreateFAdd(lhs, rhs));

    // subtraction
    f = create_llvm_function(table, module, "-", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateSub(lhs, rhs));

    // equality
    f = create_llvm_function(table, module, "==", 0);
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
