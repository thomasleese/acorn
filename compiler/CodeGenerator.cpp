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
#include "Errors.h"
#include "Mangler.h"
#include "SymbolTable.h"

#include "CodeGenerator.h"

CodeGenerator::CodeGenerator(SymbolTable::Namespace *rootNamespace) {
    m_scope = rootNamespace;

    m_irBuilder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    m_mdBuilder = new llvm::MDBuilder(llvm::getGlobalContext());
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

    SymbolTable::Symbol *symbol = m_scope->lookup(expression);

    if (!symbol->value) {
        throw Errors::InternalError(expression, "should not be nullptr");
    }

    llvm::Value *value = m_irBuilder->CreateLoad(symbol->value);
    m_llvmValues.push_back(value);
}

void CodeGenerator::visit(AST::Type *type) {
    std::string name = type->name->name;
    m_scope->lookup(type, name);
}

void CodeGenerator::visit(AST::BooleanLiteral *boolean) {
    throw Errors::InternalError(boolean, "N/A");
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
    debug("Making float literal: " + expression->value);

    llvm::Type *type = expression->type->create_llvm_type(llvm::getGlobalContext());

    double floatValue;
    std::stringstream ss;
    ss << expression->value;
    ss >> floatValue;

    llvm::Constant *value = llvm::ConstantFP::get(type, floatValue);
    m_llvmValues.push_back(value);
}

void CodeGenerator::visit(AST::ImaginaryLiteral *imaginary) {
    throw Errors::InternalError(imaginary, "N/A");
}

void CodeGenerator::visit(AST::StringLiteral *expression) {
    throw Errors::InternalError(expression, "N/A");
}

void CodeGenerator::visit(AST::SequenceLiteral *sequence) {
    throw Errors::InternalError(sequence, "N/A");
}

void CodeGenerator::visit(AST::MappingLiteral *mapping) {
    throw Errors::InternalError(mapping, "N/A");
}

void CodeGenerator::visit(AST::Argument *argument) {
    argument->value->accept(this);
}

void CodeGenerator::visit(AST::Call *expression) {
    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->operand);
    if (identifier) {
        debug("Generating a call to: " + identifier->name);

        SymbolTable::Symbol *functionSymbol = m_scope->lookup(identifier);
        Types::Function *functionType = static_cast<Types::Function *>(functionSymbol->type);

        Types::Method *method = functionType->find_method(expression, expression->arguments);

        std::string method_name = Mangler::mangle_method(functionSymbol->name, method);
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

void CodeGenerator::visit(AST::CCall *ccall) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    std::vector<llvm::Type *> parameters;
    llvm::Type *returnType = ccall->returnType->type->create_llvm_type(context);

    for (auto parameter : ccall->parameters) {
        parameters.push_back(parameter->type->create_llvm_type(context));
    }

    llvm::FunctionType *functionType = llvm::FunctionType::get(returnType, parameters, false);
    llvm::Function *function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ccall->name->name, m_module);

    std::vector<llvm::Value *> arguments;
    for (auto argument : ccall->arguments) {
        argument->accept(this);

        llvm::Value *argValue = m_llvmValues.back();
        m_llvmValues.pop_back();
        arguments.push_back(argValue);
    }

    llvm::Value *value = m_irBuilder->CreateCall(function, arguments);
    m_llvmValues.push_back(value);
}

void CodeGenerator::visit(AST::Assignment *expression) {
    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->lhs);
    if (identifier) {
        expression->rhs->accept(this);
        llvm::Value *value = m_llvmValues.back();
        m_llvmValues.pop_back();

        llvm::Value *ptr = m_scope->lookup(identifier)->value;

        m_irBuilder->CreateStore(value, ptr);
    } else {
        throw Errors::InternalError(expression, "not an identifier");
    }
}

void CodeGenerator::visit(AST::Selector *expression) {

}

void CodeGenerator::visit(AST::Comma *expression) {
    throw Errors::InternalError(expression, "N/A");
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

void CodeGenerator::visit(AST::Parameter *parameter) {
    throw Errors::InternalError(parameter, "N/A");
}

void CodeGenerator::visit(AST::VariableDefinition *definition) {
    SymbolTable::Symbol *symbol = m_scope->lookup(definition->name);

    debug("Defining a new variable: " + symbol->name);

    if (m_scope->is_root()) {
        llvm::Type *type = definition->type->create_llvm_type(llvm::getGlobalContext());

        llvm::GlobalVariable *variable = new llvm::GlobalVariable(*m_module, type, false,
                                                                  llvm::GlobalValue::CommonLinkage,
                                                                  nullptr, definition->name->name);
        variable->setAlignment(4);
        variable->setVisibility(llvm::GlobalValue::DefaultVisibility);
        variable->setInitializer(definition->type->create_llvm_initialiser(llvm::getGlobalContext()));

        symbol->value = variable;

        llvm::Function *function = m_module->getFunction("_init_variables_");
        m_irBuilder->SetInsertPoint(&function->getEntryBlock());

        definition->expression->accept(this);

        llvm::Value *value = m_llvmValues.back();
        m_llvmValues.pop_back();
        m_irBuilder->CreateStore(value, variable);

        llvm::Function *mainFunction = m_module->getFunction("main");
        m_irBuilder->SetInsertPoint(&mainFunction->getEntryBlock());
    } else {
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
}

void CodeGenerator::visit(AST::FunctionDefinition *definition) {
    SymbolTable::Symbol *functionSymbol = m_scope->lookup(definition->name);

    Types::Method *method = static_cast<Types::Method *>(definition->type);

    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup(definition, method->mangled_name());

    std::string llvm_function_name = Mangler::mangle_method(functionSymbol->name, method);

    SymbolTable::Namespace *oldNamespace = m_scope;
    m_scope = symbol->nameSpace;

    llvm::Type *llvmType = method->create_llvm_type(llvm::getGlobalContext());

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(llvmType);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_function_name, m_module);

    // function->setGC("shadow-stack");

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    m_irBuilder->SetInsertPoint(basicBlock);

    int i = 0;
    for (auto &arg : function->args()) {
        std::string name2 = method->get_parameter_name(i);
        llvm::AllocaInst *alloca = m_irBuilder->CreateAlloca(arg.getType(), 0, name2);
        m_irBuilder->CreateStore(&arg, alloca);
        SymbolTable::Symbol *symbol2 = m_scope->lookup(definition, name2);
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
            m_irBuilder->CreateRetVoid();
            // m_irBuilder->CreateRet(value);
        }
    }

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
        function->dump();
        throw Errors::InternalError(definition, stream.str());
    }

    m_scope = oldNamespace;

    llvm::Function *mainFunction = m_module->getFunction("main");
    m_irBuilder->SetInsertPoint(&mainFunction->getEntryBlock());
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

void CodeGenerator::visit(AST::ImportStatement *statement) {
    throw Errors::InternalError(statement, "N/A");
}

void CodeGenerator::visit(AST::SourceFile *module) {
    m_module = new llvm::Module(module->name, llvm::getGlobalContext());

    Builtins::fill_llvm_module(m_scope, m_module, m_irBuilder);

    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "_init_variables_", m_module);
    llvm::BasicBlock *bb1 = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);

    fType = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), false);
    llvm::Function *mainFunction = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "main", m_module);
    llvm::BasicBlock *main_bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction);

    m_irBuilder->SetInsertPoint(main_bb);
    m_irBuilder->CreateCall(function);

    module->code->accept(this);

    m_irBuilder->SetInsertPoint(bb1);
    m_irBuilder->CreateRetVoid();

    m_irBuilder->SetInsertPoint(main_bb);
    m_irBuilder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), 0));

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyModule(*m_module, &stream)) {
        m_module->dump();
        throw Errors::InternalError(module, stream.str());
    }
}
