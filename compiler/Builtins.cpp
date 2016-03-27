//
// Created by Thomas Leese on 18/03/2016.
//

#include <sstream>

#include "SymbolTable.h"
#include "Types.h"

#include "Builtins.h"

SymbolTable::Symbol *add_symbol(SymbolTable::Namespace *table, std::string name, Types::Type *type) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    symbol->type = type;
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
    std::stringstream ss;
    ss << function->nameSpace->size();

    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(ss.str());
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
    add_symbol(table, "Float16", new Types::FloatConstructor(16));
    add_symbol(table, "Float32", new Types::FloatConstructor(32));
    add_symbol(table, "Float64", new Types::FloatConstructor(64));
    add_symbol(table, "Float128", new Types::FloatConstructor(128));
    add_symbol(table, "Sequence", new Types::SequenceConstructor());
    add_symbol(table, "Function", new Types::FunctionConstructor());
    add_symbol(table, "Union", new Types::UnionConstructor());
}

void Builtins::fill_symbol_table(SymbolTable::Namespace *table) {
    add_base_type_constructors(table);

    add_symbol(table, "Nothing", new Types::Void());

    SymbolTable::Symbol *function = add_base_function(table, "_debug_print_");
    add_base_method(function, new Types::Method("x", new Types::Integer(64), new Types::Void()));

    SymbolTable::Symbol *multiplication = add_base_function(table, "*");
    add_base_method(multiplication, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));

    SymbolTable::Symbol *addition = add_base_function(table, "+");
    add_base_method(addition, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));

    SymbolTable::Symbol *subtraction = add_base_function(table, "-");
    add_base_method(subtraction, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));

    SymbolTable::Symbol *equality = add_base_function(table, "==");
    add_base_method(equality, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Boolean()));

    SymbolTable::Symbol *not_equality = add_base_function(table, "!=");
    add_base_method(not_equality, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Boolean()));

    SymbolTable::Symbol *less_than = add_base_function(table, "<");
    add_base_method(less_than, new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Boolean()));
}

void Builtins::fill_llvm_module(SymbolTable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder) {
    Types::Function *functionType = static_cast<Types::Function *>(table->lookup(nullptr, "*")->type);
    Types::Method *methodType = functionType->get_method(0);

    llvm::LLVMContext &context = module->getContext();

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    llvm::Function *f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "*_0", module);

    llvm::Argument *lhs = &f->getArgumentList().front();
    llvm::Argument *rhs = &f->getArgumentList().back();

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateMul(lhs, rhs));

    // addition
    functionType = static_cast<Types::Function *>(table->lookup(nullptr, "+")->type);
    methodType = functionType->get_method(0);

    type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "+_0", module);

    lhs = &f->getArgumentList().front();
    rhs = &f->getArgumentList().back();

    basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateAdd(lhs, rhs));

    // subtraction
    functionType = static_cast<Types::Function *>(table->lookup(nullptr, "-")->type);
    methodType = functionType->get_method(0);

    type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "-_0", module);

    lhs = &f->getArgumentList().front();
    rhs = &f->getArgumentList().back();

    basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateSub(lhs, rhs));

    // equality
    functionType = static_cast<Types::Function *>(table->lookup(nullptr, "==")->type);
    methodType = functionType->get_method(0);

    type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "==_0", module);

    lhs = &f->getArgumentList().front();
    rhs = &f->getArgumentList().back();

    basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateICmpEQ(lhs, rhs));

    // not equality
    functionType = static_cast<Types::Function *>(table->lookup(nullptr, "!=")->type);
    methodType = functionType->get_method(0);

    type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "!=_0", module);

    lhs = &f->getArgumentList().front();
    rhs = &f->getArgumentList().back();

    basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateICmpNE(lhs, rhs));

    // less than
    functionType = static_cast<Types::Function *>(table->lookup(nullptr, "<")->type);
    methodType = functionType->get_method(0);

    type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "<_0", module);

    lhs = &f->getArgumentList().front();
    rhs = &f->getArgumentList().back();

    basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateICmpSLT(lhs, rhs));
}
