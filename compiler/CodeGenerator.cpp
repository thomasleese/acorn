//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>

#include "Builtins.h"
#include "SymbolTable.h"
#include "Errors.h"

#include "CodeGenerator.h"

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
    // std::cerr << line << std::endl;
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

    if (!symbol->value) {
        throw Errors::InternalError(expression, "should not be nullptr");
    }

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

        SymbolTable::Symbol *functionSymbol = m_namespace->lookup(identifier);
        Types::Function *functionType = static_cast<Types::Function *>(functionSymbol->type);

        Types::Method *method = functionType->find_method(expression, expression->arguments);

        SymbolTable::Symbol *methodSymbol = nullptr;
        for (int i = 0; i < functionSymbol->nameSpace->size(); i++) {
            std::stringstream ss;
            ss << i;
            SymbolTable::Symbol *m = functionSymbol->nameSpace->lookup(expression, ss.str());
            if (m->type == method) {
                methodSymbol = m;
            }
        }
        assert(methodSymbol);

        std::string method_name = functionSymbol->name + "_" + methodSymbol->name;

        llvm::Function *function = m_module->getFunction(method_name);
        if (!function) {
            throw Errors::InternalError(expression, "No function defined (" + method_name + ").");
        }

        const unsigned long noArguments = expression->arguments.size();

        std::vector<llvm::Value *> arguments;
        for (unsigned long i = 0; i < noArguments; i++) {
            arguments.push_back(nullptr);
        }

        for (unsigned long i = 0; i < expression->arguments.size(); i++) {
            AST::Argument *argument = expression->arguments[i];

            std::stringstream ss;
            ss << "Argument " << i;
            debug(ss.str());

            argument->accept(this);

            long index = i;
            if (argument->name) {
                debug("Getting parameter position of " + argument->name->name);
                index = method->get_parameter_position(argument->name->name);
                if (index < 0) {
                    throw Errors::InternalError(argument, "no argument");
                }
            }

            ss = std::stringstream();
            ss << "Assigned to index " << index;
            debug(ss.str());

            arguments[index] = m_llvmValues.back();
            m_llvmValues.pop_back();
        }

        debug("Creating call instruction");

        llvm::Value *value = m_irBuilder->CreateCall(function, arguments);
        m_llvmValues.push_back(value);

        debug("Ending call to: " + identifier->name);
    } else {
        throw Errors::InternalError(expression, "Not an identifier.");
    }
}

void CodeGenerator::visit(AST::Assignment *expression) {
    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->lhs);
    if (identifier) {
        expression->rhs->accept(this);
        llvm::Value *value = m_llvmValues.back();
        m_llvmValues.pop_back();

        llvm::Value *ptr = m_namespace->lookup(identifier)->value;

        m_irBuilder->CreateStore(value, ptr);
    } else {
        throw Errors::InternalError(expression, "not an identifier");
    }
}

void CodeGenerator::visit(AST::Selector *expression) {

}

void CodeGenerator::visit(AST::Comma *expression) {

}

void CodeGenerator::visit(AST::While *expression) {
    debug("Creating while statement...");

    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::Function *function = m_irBuilder->GetInsertBlock()->getParent();

    llvm::BasicBlock *while_bb = llvm::BasicBlock::Create(context, "while", function);

    m_irBuilder->CreateBr(while_bb);

    m_irBuilder->SetInsertPoint(while_bb);

    expression->condition->accept(this);

    llvm::Value *condition = m_llvmValues.back();
    m_llvmValues.pop_back();

    condition = m_irBuilder->CreateICmpEQ(condition, llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1), "whilecond");

    llvm::BasicBlock *then_bb = llvm::BasicBlock::Create(context, "whilethen", function);
    llvm::BasicBlock *merge_bb = llvm::BasicBlock::Create(context, "whilecont");

    m_irBuilder->CreateCondBr(condition, then_bb, merge_bb);
    m_irBuilder->SetInsertPoint(then_bb);

    debug("Generating loop code...");
    expression->code->accept(this);
    //llvm::Value *then_value = m_llvmValues.back();
    m_llvmValues.pop_back();

    m_irBuilder->CreateBr(while_bb);

    then_bb = m_irBuilder->GetInsertBlock();

    function->getBasicBlockList().push_back(merge_bb);
    m_irBuilder->SetInsertPoint(merge_bb);

    /*llvm::Type *type = expression->type->create_llvm_type(context);
    llvm::PHINode *phi = m_irBuilder->CreatePHI(type, 2, "iftmp");
    phi->addIncoming(then_value, then_bb);*/

    m_llvmValues.push_back(then_bb);

    debug("Ended if statement.");
}

void CodeGenerator::visit(AST::For *expression) {
    throw Errors::InternalError(expression, "Should not be in the lowered AST.");
}

void CodeGenerator::visit(AST::If *expression) {
    debug("Creating if statement...");

    expression->condition->accept(this);

    llvm::Value *condition = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::LLVMContext &context = llvm::getGlobalContext();

    condition = m_irBuilder->CreateICmpEQ(condition, llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1), "ifcond");

    llvm::Function *function = m_irBuilder->GetInsertBlock()->getParent();

    llvm::BasicBlock *then_bb = llvm::BasicBlock::Create(context, "then", function);
    llvm::BasicBlock *else_bb = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock *merge_bb = llvm::BasicBlock::Create(context, "ifcont");

    m_irBuilder->CreateCondBr(condition, then_bb, else_bb);
    m_irBuilder->SetInsertPoint(then_bb);

    debug("Generating true code...");
    expression->trueCode->accept(this);
    llvm::Value *then_value = m_llvmValues.back();
    m_llvmValues.pop_back();

    m_irBuilder->CreateBr(merge_bb);

    then_bb = m_irBuilder->GetInsertBlock();

    function->getBasicBlockList().push_back(else_bb);
    m_irBuilder->SetInsertPoint(else_bb);

    llvm::Value *else_value;
    if (expression->falseCode) {
        debug("Generating false code...");
        expression->falseCode->accept(this);
        else_value = m_llvmValues.back();
        m_llvmValues.pop_back();
    } else {
        throw Errors::InternalError(expression, "no else");
    }

    m_irBuilder->CreateBr(merge_bb);

    else_bb = m_irBuilder->GetInsertBlock();

    function->getBasicBlockList().push_back(merge_bb);
    m_irBuilder->SetInsertPoint(merge_bb);

    llvm::Type *type = expression->type->create_llvm_type(context);
    llvm::PHINode *phi = m_irBuilder->CreatePHI(type, 2, "iftmp");
    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    m_llvmValues.push_back(phi);

    debug("Ended if statement.");
}

void CodeGenerator::visit(AST::Return *expression) {
    debug("Generating return statement.");

    expression->expression->accept(this);

    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Value *returnValue = m_irBuilder->CreateRet(value);
    m_llvmValues.push_back(returnValue);
}

void CodeGenerator::visit(AST::Spawn *expression) {
    debug("Generating spawn statement.");
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
    SymbolTable::Symbol *functionSymbol = m_namespace->lookup(definition->name);

    Types::Method *method = static_cast<Types::Method *>(definition->type);

    SymbolTable::Symbol *symbol = nullptr;
    for (int i = 0; i < functionSymbol->nameSpace->size(); i++) {
        std::stringstream ss;
        ss << i;
        SymbolTable::Symbol *s = functionSymbol->nameSpace->lookup(definition, ss.str());
        if (s->type == method && s->node == definition) {
            symbol = s;
        }
    }
    assert(symbol);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    llvm::Type *llvmType = method->create_llvm_type(llvm::getGlobalContext());

    // the method name is the function name plus the method name
    std::string method_name = functionSymbol->name + "_" + symbol->name;

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(llvmType);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, method_name, m_module);

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    m_irBuilder->SetInsertPoint(basicBlock);

    int i = 0;
    for (auto &arg : function->args()) {
        std::string name2 = method->get_parameter_name(i);
        llvm::AllocaInst *alloca = m_irBuilder->CreateAlloca(arg.getType(), 0, name2);
        m_irBuilder->CreateStore(&arg, alloca);
        SymbolTable::Symbol *symbol2 = m_namespace->lookup(definition, name2);
        symbol2->value = alloca;
        i++;
    }

    definition->code->accept(this);

    if (definition->code->statements.empty()) {
        m_irBuilder->CreateRetVoid();
    } else {
        llvm::Value *value = m_llvmValues.back();
        m_llvmValues.pop_back();
        if (!llvm::isa<llvm::ReturnInst>(value)) {
            m_irBuilder->CreateRet(value);
        }
    }

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
        throw Errors::InternalError(definition, stream.str());
    }

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
    debug("Generating definition statement.");
    statement->definition->accept(this);
}

void CodeGenerator::visit(AST::ExpressionStatement *statement) {
    debug("Generating expression statement.");
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

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyModule(*m_module, &stream)) {
        throw Errors::InternalError(module, stream.str());
    }

    m_namespace = oldNamespace;
}
