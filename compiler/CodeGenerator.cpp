//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "SymbolTable.h"

#include "CodeGenerator.h"

CodeGenerator::CodeGenerator(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;

    m_irBuilder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    m_mdBuilder = new llvm::MDBuilder(llvm::getGlobalContext());
}

CodeGenerator::~CodeGenerator() {
    delete m_irBuilder;
    delete m_mdBuilder;
}

void CodeGenerator::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void CodeGenerator::visit(AST::Identifier *expression) {

}

void CodeGenerator::visit(AST::BooleanLiteral *boolean) {

}

void CodeGenerator::visit(AST::IntegerLiteral *expression) {

}

void CodeGenerator::visit(AST::FloatLiteral *expression) {

}

void CodeGenerator::visit(AST::ImaginaryLiteral *imaginary) {

}

void CodeGenerator::visit(AST::StringLiteral *expression) {

}

void CodeGenerator::visit(AST::SequenceLiteral *sequence) {

}

void CodeGenerator::visit(AST::MappingLiteral *mapping) {

}

void CodeGenerator::visit(AST::Argument *argument) {

}

void CodeGenerator::visit(AST::Call *expression) {

}

void CodeGenerator::visit(AST::Assignment *expression) {

}

void CodeGenerator::visit(AST::Selector *expression) {

}

void CodeGenerator::visit(AST::While *expression) {

}

void CodeGenerator::visit(AST::For *expression) {

}

void CodeGenerator::visit(AST::If *expression) {

}

void CodeGenerator::visit(AST::Type *type) {
    std::string name = type->name->name;
    m_namespace->lookup(type, name);
}

void CodeGenerator::visit(AST::Cast *cast) {
    cast->typeNode->accept(this);
}

void CodeGenerator::visit(AST::Parameter *parameter) {

}

void CodeGenerator::visit(AST::VariableDefinition *definition) {
    definition->cast->accept(this);

    llvm::Type *type = llvm::Type::getInt64Ty(llvm::getGlobalContext());

    bool isConstant = true;
    llvm::GlobalVariable *variable = new llvm::GlobalVariable(type, isConstant, llvm::GlobalValue::ExternalLinkage, 0, definition->name->name);
    variable->setAlignment(4);

    variable->setVisibility(llvm::GlobalValue::DefaultVisibility);

    variable->setInitializer(llvm::ConstantInt::get(type, 0, true));
}

void CodeGenerator::visit(AST::FunctionDefinition *definition) {
    std::string name = definition->name->name;

    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    llvm::FunctionType *type = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, m_module);

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    m_irBuilder->SetInsertPoint(basicBlock);

    definition->code->accept(this);

    m_namespace = oldNamespace;
}

void CodeGenerator::visit(AST::TypeDefinition *definition) {
    std::string name = definition->name->name->name;

    /*std::vector<llvm::TypeType *> elements;

    for (auto field : definition->fields) {
        std::cout << field << std::endl;
        elements.push_back(llvm::IntegerType::getInt64Ty(llvm::getGlobalContext()));
    }

    llvm::StructType::create(elements, name);*/
}

void CodeGenerator::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void CodeGenerator::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void CodeGenerator::visit(AST::Module *module) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(module, module->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    m_module = new llvm::Module(module->name, llvm::getGlobalContext());

    module->code->accept(this);

    m_namespace = oldNamespace;

    m_module->dump();
    delete m_module;
}
