//
// Created by Thomas Leese on 18/03/2016.
//

#include <iostream>
#include <sstream>

#include <llvm/IR/Module.h>

#include "symboltable.h"
#include "typing/types.h"
#include "codegen/module.h"
#include "codegen/types.h"

#include "builtins.h"

using namespace acorn;
using namespace acorn::builtins;

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
    add_symbol(table, "Any", new types::AnyType());
    add_symbol(table, "Void", new types::VoidType());
    add_symbol(table, "Boolean", new types::BooleanType());
    add_symbol(table, "Integer8", new types::IntegerType(8));
    add_symbol(table, "Integer16", new types::IntegerType(16));
    add_symbol(table, "Integer32", new types::IntegerType(32));
    add_symbol(table, "Integer64", new types::IntegerType(64));
    add_symbol(table, "Integer128", new types::IntegerType(128));
    add_symbol(table, "UnsignedInteger8", new types::UnsignedIntegerType(8));
    add_symbol(table, "UnsignedInteger16", new types::UnsignedIntegerType(16));
    add_symbol(table, "UnsignedInteger32", new types::UnsignedIntegerType(32));
    add_symbol(table, "UnsignedInteger64", new types::UnsignedIntegerType(64));
    add_symbol(table, "UnsignedInteger128", new types::UnsignedIntegerType(128));
    add_symbol(table, "Float16", new types::FloatType(16));
    add_symbol(table, "Float32", new types::FloatType(32));
    add_symbol(table, "Float64", new types::FloatType(64));
    add_symbol(table, "Float128", new types::FloatType(128));
    add_symbol(table, "UnsafePointer", new types::UnsafePointerType());
    add_symbol(table, "Function", new types::FunctionType());
    add_symbol(table, "Union", new types::UnionType());
    add_symbol(table, "Tuple", new types::TupleType());
    add_symbol(table, "Type", new types::TypeDescriptionType());
}

void builtins::fill_symbol_table(symboltable::Namespace *table) {
    add_base_type_constructors(table);

    add_symbol(table, "Nothing", new types::Void());
    add_symbol(table, "True", new types::Boolean());
    add_symbol(table, "False", new types::Boolean());

    auto not_ = add_base_function(table, "not");
    add_base_method(not_, new types::Method(new types::Boolean(), new types::Boolean()));

    symboltable::Symbol *multiplication = add_base_function(table, "*");
    add_base_method(multiplication, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    symboltable::Symbol *addition = add_base_function(table, "+");
    add_base_method(addition, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));
    add_base_method(addition, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::UnsignedInteger(64)));
    add_base_method(addition, new types::Method(new types::Float(64), new types::Float(64), new types::Float(64)));

    symboltable::Symbol *subtraction = add_base_function(table, "-");
    add_base_method(subtraction, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    symboltable::Symbol *equality = add_base_function(table, "==");
    add_base_method(equality, new types::Method(new types::Boolean(), new types::Boolean(), new types::Boolean()));
    add_base_method(equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));
    add_base_method(equality, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::Boolean()));

    symboltable::Symbol *not_equality = add_base_function(table, "!=");
    add_base_method(not_equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    symboltable::Symbol *less_than = add_base_function(table, "<");
    add_base_method(less_than, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    auto gte = add_base_function(table, ">=");
    add_base_method(gte, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    symboltable::Symbol *to_integer = add_base_function(table, "to_integer");
    add_base_method(to_integer, new types::Method(new types::Float(64), new types::Integer(64)));

    symboltable::Symbol *to_float = add_base_function(table, "to_float");
    add_base_method(to_float, new types::Method(new types::Integer(64), new types::Float(64)));

    auto getindex = add_base_function(table, "getindex");
    auto pc = new types::ParameterType();
    auto method = new types::Method(new types::UnsafePointer(new types::Parameter(pc)), new types::Integer(64), new types::Parameter(pc));
    method->set_is_generic(true);
    add_base_method(getindex, method);

    auto setindex = add_base_function(table, "setindex");
    pc = new types::ParameterType();
    method = new types::Method(new types::UnsafePointer(new types::Parameter(pc)), new types::Integer(64), new types::Parameter(pc), new types::Void());
    method->set_is_generic(true);
    add_base_method(setindex, method);
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
static llvm::Argument *a;
static llvm::Argument *c;
static llvm::Argument *b;

void initialise_function_block(llvm::Function *function, llvm::IRBuilder<> *irBuilder) {
    llvm::LLVMContext &context = function->getContext();

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(context, "entry", function);
    irBuilder->SetInsertPoint(basicBlock);
}

void initialise_ternary_function(llvm::Function *function, llvm::IRBuilder<> *irBuilder) {
    a = &function->getArgumentList().front();
    b = function->getArgumentList().getNext(a);
    c = function->getArgumentList().getNext(b);

    initialise_function_block(function, irBuilder);
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

void initialiser_boolean_variable(symboltable::Namespace *table, std::string name, llvm::Module *module, llvm::IRBuilder<> *irBuilder, bool value) {
    auto sym = table->lookup(nullptr, nullptr, name);
    sym->value = new llvm::GlobalVariable(*module, irBuilder->getInt1Ty(), false,
                                          llvm::GlobalValue::InternalLinkage,
                                          irBuilder->getInt1(value), name);
}

void builtins::fill_llvm_module(symboltable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder) {
    initialiser_boolean_variable(table, "Nothing", module, irBuilder, false);
    initialiser_boolean_variable(table, "True", module, irBuilder, true);
    initialiser_boolean_variable(table, "False", module, irBuilder, false);
    initialiser_boolean_variable(table, "Integer32", module, irBuilder, false);
    initialiser_boolean_variable(table, "Integer64", module, irBuilder, false);

    // not
    llvm::Function *f = create_llvm_function(table, module, "not", 0);
    initialise_unary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateNot(self));

    // multiplication
    f = create_llvm_function(table, module, "*", 0);
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
    for (int i = 0; i < 3; i++) {
        f = create_llvm_function(table, module, "==", 0);
        initialise_binary_function(f, irBuilder);
        irBuilder->CreateRet(irBuilder->CreateICmpEQ(lhs, rhs));
    }

    // not equality
    f = create_llvm_function(table, module, "!=", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpNE(lhs, rhs));

    // less than
    f = create_llvm_function(table, module, "<", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpSLT(lhs, rhs));

    // greater than or equal to
    f = create_llvm_function(table, module, ">=", 0);
    initialise_binary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateICmpSGE(lhs, rhs));

    // to integer
    f = create_llvm_function(table, module, "to_integer", 0);
    initialise_unary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateFPToSI(self, f->getReturnType()));

    // to integer
    f = create_llvm_function(table, module, "to_float", 0);
    initialise_unary_function(f, irBuilder);
    irBuilder->CreateRet(irBuilder->CreateSIToFP(self, f->getReturnType()));
}

void builtins::generate_function(symboltable::Symbol *function_symbol, symboltable::Symbol *method_symbol,
                                 types::Method *method,
                                 std::string llvm_name, llvm::Module *module, llvm::IRBuilder<> *irBuilder,
                                 codegen::TypeGenerator *type_generator) {
    method->accept(type_generator);

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(type_generator->take_type(nullptr));
    assert(type);

    llvm::Function *f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_name, module);

    auto old_insert_point = irBuilder->saveIP();

    initialise_ternary_function(f, irBuilder);
    std::vector<llvm::Value *> index_list;
    index_list.push_back(b);
    auto gep = irBuilder->CreateInBoundsGEP(a, index_list);

    if (function_symbol->name == "setindex") {
        irBuilder->CreateStore(c, gep);
        irBuilder->CreateRet(irBuilder->getInt1(false));
    } else {
        irBuilder->CreateRet(irBuilder->CreateLoad(gep));
    }

    irBuilder->restoreIP(old_insert_point);
}
