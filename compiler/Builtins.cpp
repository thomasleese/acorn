//
// Created by Thomas Leese on 18/03/2016.
//

#include "SymbolTable.h"
#include "Types.h"

#include "Builtins.h"

SymbolTable::Symbol *add_symbol(SymbolTable::Namespace *table, std::string name, Types::Type *type) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    symbol->type = type;
    table->insert(nullptr, symbol);
    return symbol;
}

Types::Function *add_base_function(SymbolTable::Namespace *table, std::string name) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    Types::Function *type = new Types::Function();
    symbol->type = type;
    table->insert(nullptr, symbol);
    return type;
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

    Types::Function *function = add_base_function(table, "_debug_print_");
    function->add_method(new Types::Method("x", new Types::Integer(64), new Types::Void()));

    Types::Function *multiplication = add_base_function(table, "*");
    multiplication->add_method(new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));

    Types::Function *addition = add_base_function(table, "+");
    addition->add_method(new Types::Method("a", new Types::Integer(64), "b", new Types::Integer(64), new Types::Integer(64)));
}

void Builtins::fill_llvm_module(SymbolTable::Namespace *table, llvm::Module *module, llvm::IRBuilder<> *irBuilder) {
    Types::Function *functionType = static_cast<Types::Function *>(table->lookup(nullptr, "*")->type);
    Types::Method *methodType = functionType->get_method(0);

    llvm::LLVMContext &context = module->getContext();

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    llvm::Function *f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, methodType->llvm_name("*"), module);

    llvm::Argument *lhs = &f->getArgumentList().front();
    llvm::Argument *rhs = &f->getArgumentList().back();

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateMul(lhs, rhs));

    type = static_cast<llvm::FunctionType *>(methodType->create_llvm_type(context));
    f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, methodType->llvm_name("+"), module);

    lhs = &f->getArgumentList().front();
    rhs = &f->getArgumentList().back();

    basicBlock = llvm::BasicBlock::Create(context, "entry", f);
    irBuilder->SetInsertPoint(basicBlock);
    irBuilder->CreateRet(irBuilder->CreateAdd(lhs, rhs));
}
