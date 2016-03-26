//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "Builtins.h"
#include "SymbolTable.h"

#include "CodeGenerator.h"
#include "Errors.h"

CodeGenerator::CodeGenerator(SymbolTable::Namespace *rootNamespace, llvm::TargetMachine *target_machine) {
    m_namespace = rootNamespace;

    m_irBuilder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    m_mdBuilder = new llvm::MDBuilder(llvm::getGlobalContext());

    m_target_machine = target_machine;
}

CodeGenerator::~CodeGenerator() {
    delete m_irBuilder;
    delete m_mdBuilder;
}

void CodeGenerator::debug(std::string line) {
    std::cerr << line << std::endl;
}

llvm::Module *CodeGenerator::module() const {
    return m_module;
}

void CodeGenerator::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void CodeGenerator::visit(AST::Identifier *expression) {
    debug("Finding named value: " + expression->name);
    SymbolTable::Symbol *symbol = m_namespace->lookup(expression);
    llvm::Value *value = m_irBuilder->CreateLoad(symbol->value);
    m_llvmValues.push_back(value);
}

void CodeGenerator::visit(AST::BooleanLiteral *boolean) {

}

void CodeGenerator::visit(AST::IntegerLiteral *expression) {
    debug("Making integer literal: " + expression->value);
    llvm::Type *type = expression->type->create_llvm_type(llvm::getGlobalContext());

    int integer;
    std::stringstream ss;
    ss << expression->value;
    ss >> integer;

    llvm::Constant *value = llvm::ConstantInt::get(type, integer, true);
    m_llvmValues.push_back(value);
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
    argument->value->accept(this);
}

void CodeGenerator::visit(AST::Call *expression) {
    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->operand);
    if (identifier) {
        debug("Generating a call to: " + identifier->name);

        m_namespace->lookup(identifier);

        Types::Method *method = static_cast<Types::Method *>(expression->type);
        std::string functionName = method->llvm_name(identifier->name);

        llvm::Function *function = m_module->getFunction(functionName);

        const unsigned long noArguments = expression->arguments.size();

        std::vector<llvm::Value *> arguments;
        for (unsigned long i = 0; i < noArguments; i++) {
            arguments.push_back(nullptr);
        }

        for (unsigned long i = 0; i < expression->arguments.size(); i++) {
            AST::Argument *argument = expression->arguments[i];

            argument->accept(this);

            unsigned long index = i;
            if (argument->name) {
                index = method->get_parameter_position(argument->name->name);
            }

            llvm::Value *value = m_llvmValues.back();
            arguments[index] = value;
            m_llvmValues.pop_back();

            std::cerr << value->getType() << " " << value->getType()->isIntegerTy(64) << " " << function->getFunctionType()->getParamType(i)->isIntegerTy(64) << std::endl;
        }

        llvm::Value *value = m_irBuilder->CreateCall(function, arguments);
        m_llvmValues.push_back(value);
    } else {
        throw Errors::InternalError(expression, "Not an identifier.");
    }
}

void CodeGenerator::visit(AST::Assignment *expression) {

}

void CodeGenerator::visit(AST::Selector *expression) {

}

void CodeGenerator::visit(AST::Comma *expression) {

}

void CodeGenerator::visit(AST::While *expression) {

}

void CodeGenerator::visit(AST::For *expression) {

}

void CodeGenerator::visit(AST::If *expression) {

}

void CodeGenerator::visit(AST::Return *expression) {
    expression->expression->accept(this);

    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    m_irBuilder->CreateRet(value);
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
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition->name);

    debug("Defining a new variable: " + symbol->name);

    /*llvm::Type *type = definition->type->create_llvm_type(llvm::getGlobalContext());
    bool isConstant = !definition->is_mutable;

    llvm::GlobalVariable *variable = new llvm::GlobalVariable(type, isConstant, llvm::GlobalValue::ExternalLinkage, 0, definition->name->name);
    variable->setAlignment(4);

    symbol->value = variable;

    variable->setVisibility(llvm::GlobalValue::DefaultVisibility);

    definition->expression->accept(this);

    llvm::Constant *initialiser = static_cast<llvm::Constant *>(m_llvmValues.back());
    m_llvmValues.pop_back();
    variable->setInitializer(initialiser);

    // need to find a way around this
    symbol->value = initialiser;*/

    llvm::Function *function = m_irBuilder->GetInsertBlock()->getParent();

    definition->expression->accept(this);

    llvm::Value *initialiser = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Type *type = definition->type->create_llvm_type(llvm::getGlobalContext());

    llvm::IRBuilder<> tmp_ir(&function->getEntryBlock(), function->getEntryBlock().begin());
    llvm::AllocaInst *variable = tmp_ir.CreateAlloca(type, 0, definition->name->name);

    m_irBuilder->CreateStore(initialiser, variable);

    symbol->value = variable;
}

void CodeGenerator::visit(AST::FunctionDefinition *definition) {
    std::string name = definition->name->name;

    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    Types::Method *methodType = static_cast<Types::Method *>(definition->type);

    llvm::Type *llvmType = methodType->create_llvm_type(llvm::getGlobalContext());

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(llvmType);
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
    m_module->setDataLayout(m_target_machine->createDataLayout());

    Builtins::fill_llvm_module(m_namespace, m_module, m_irBuilder);

    module->code->accept(this);

    m_namespace = oldNamespace;
}
