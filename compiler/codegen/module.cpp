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

#include "../Builtins.h"
#include "../Errors.h"
#include "../Mangler.h"
#include "../SymbolTable.h"
#include "../Types.h"

#include "types.h"

#include "module.h"

using namespace jet::codegen;

ModuleGenerator::ModuleGenerator(SymbolTable::Namespace *rootNamespace) :
        m_type_generator(new TypeGenerator()) {
    m_scope = rootNamespace;

    m_irBuilder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    m_mdBuilder = new llvm::MDBuilder(llvm::getGlobalContext());
}

ModuleGenerator::~ModuleGenerator() {
    delete m_irBuilder;
    delete m_mdBuilder;
}

void ModuleGenerator::debug(std::string line) {
    // std::cerr << line << std::endl;
}

llvm::Module *ModuleGenerator::module() const {
    return m_module;
}

llvm::Type *ModuleGenerator::generate_type(AST::Node *node, Types::Type *type) {
    type->accept(m_type_generator);
    return m_type_generator->take_type(node);
}

llvm::Type *ModuleGenerator::generate_type(AST::Node *node) {
    return generate_type(node, node->type);
}

void ModuleGenerator::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void ModuleGenerator::visit(AST::Identifier *identifier) {
    debug("Finding named value: " + identifier->value);

    SymbolTable::Symbol *symbol = m_scope->lookup(identifier);

    if (!symbol->value) {
        throw Errors::InternalError(identifier, "should not be nullptr");
    }

    if (identifier->has_parameters()) {
        throw Errors::InternalError(identifier, "should not have parameters");
    }

    llvm::Value *value = m_irBuilder->CreateLoad(symbol->value);
    m_llvmValues.push_back(value);
}

void ModuleGenerator::visit(AST::BooleanLiteral *boolean) {
    throw Errors::InternalError(boolean, "N/A");
}

void ModuleGenerator::visit(AST::IntegerLiteral *expression) {
    debug("Making integer literal: " + expression->value);

    llvm::Type *type = generate_type(expression);

    uint64_t integer;
    std::stringstream ss;
    ss << expression->value;
    ss >> integer;

    llvm::Constant *value = llvm::ConstantInt::get(type, integer, true);
    m_llvmValues.push_back(value);
}

void ModuleGenerator::visit(AST::FloatLiteral *expression) {
    debug("Making float literal: " + expression->value);

    llvm::Type *type = generate_type(expression);

    double floatValue;
    std::stringstream ss;
    ss << expression->value;
    ss >> floatValue;

    llvm::Constant *value = llvm::ConstantFP::get(type, floatValue);
    m_llvmValues.push_back(value);
}

void ModuleGenerator::visit(AST::ImaginaryLiteral *imaginary) {
    throw Errors::InternalError(imaginary, "N/A");
}

void ModuleGenerator::visit(AST::StringLiteral *expression) {
    throw Errors::InternalError(expression, "N/A");
}

void ModuleGenerator::visit(AST::SequenceLiteral *sequence) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    std::vector<llvm::Value *> elements;

    for (auto element : sequence->elements) {
        element->accept(this);

        llvm::Value *value = m_llvmValues.back();
        m_llvmValues.pop_back();
        elements.push_back(value);
    }

    llvm::StructType *type = static_cast<llvm::StructType *>(generate_type(sequence));

    llvm::Type *length_type = type->elements()[0];
    llvm::Type *element_type = type->elements()[0];

    // assign length
    llvm::Value *instance = m_irBuilder->CreateAlloca(type, nullptr, "array");

    llvm::Type *i32 = llvm::IntegerType::getInt32Ty(context);

    std::vector<llvm::Value *> length_index;
    length_index.push_back(llvm::ConstantInt::get(i32, 0));
    length_index.push_back(llvm::ConstantInt::get(i32, 0));

    std::vector<llvm::Value *> elements_index;
    elements_index.push_back(llvm::ConstantInt::get(i32, 0));
    elements_index.push_back(llvm::ConstantInt::get(i32, 1));

    llvm::Value *length = m_irBuilder->CreateInBoundsGEP(instance, length_index, "length");
    m_irBuilder->CreateStore(llvm::ConstantInt::get(length_type, elements.size()), length);

    auto elements_value = m_irBuilder->CreateInBoundsGEP(instance, elements_index, "elements");
    auto elements_instance = m_irBuilder->CreateAlloca(element_type, llvm::ConstantInt::get(i32, elements.size()));

    for (int i = 0; i < elements.size(); i++) {
        std::vector<llvm::Value *> index;
        index.push_back(llvm::ConstantInt::get(i32, i));
        auto place = m_irBuilder->CreateInBoundsGEP(elements_instance, index);
        m_irBuilder->CreateStore(elements[i], place);
    }

    m_irBuilder->CreateStore(elements_instance, elements_value);

    m_llvmValues.push_back(m_irBuilder->CreateLoad(instance));
}

void ModuleGenerator::visit(AST::MappingLiteral *mapping) {
    throw Errors::InternalError(mapping, "N/A");
}

void ModuleGenerator::visit(AST::Argument *argument) {
    argument->value->accept(this);
}

void ModuleGenerator::visit(AST::Call *expression) {
    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->operand);
    if (identifier) {
        debug("Generating a call to: " + identifier->value);

        SymbolTable::Symbol *symbol = m_scope->lookup(identifier);
        Types::Function *functionType = dynamic_cast<Types::Function *>(symbol->type);
        Types::RecordConstructor *recordType = dynamic_cast<Types::RecordConstructor *>(symbol->type);

        assert(functionType || recordType);

        llvm::Function *function;

        if (recordType == nullptr) {
            Types::Method *method = functionType->find_method(expression, expression->arguments);

            std::string method_name = Mangler::mangle_method(symbol->name, method);
            function = m_module->getFunction(method_name);
            if (!function) {
                throw Errors::InternalError(expression, "No function defined (" + method_name + ").");
            }
        } else {
            std::string function_name = identifier->value + "_new";
            function = m_module->getFunction(function_name);
            if (!function) {
                throw Errors::InternalError(expression, "No type function defined (" + function_name + ").");
            }
        }

        std::map<std::string, uint64_t> arg_positions;
        uint64_t i = 0;
        for (auto &arg : function->args()) {
            arg_positions[arg.getName()] = i;
            i++;
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

            uint64_t index = i;
            if (argument->name) {
                debug("Getting parameter position of " + argument->name->value);
                auto it = arg_positions.find(argument->name->value);
                if (it == arg_positions.end()) {
                    throw Errors::InternalError(argument, "no argument");
                } else {
                    index = it->second;
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

        debug("Ending call to: " + identifier->value);
    } else {
        throw Errors::InternalError(expression, "Not an identifier.");
    }
}

void ModuleGenerator::visit(AST::CCall *ccall) {
    std::vector<llvm::Type *> parameters;
    llvm::Type *returnType = generate_type(ccall->returnType);

    for (auto parameter : ccall->parameters) {
        parameters.push_back(generate_type(parameter));
    }

    std::string name = ccall->name->value;

    llvm::FunctionType *functionType = llvm::FunctionType::get(returnType, parameters, false);

    llvm::Function *function = m_module->getFunction(name);
    if (!function) {
        function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, m_module);
    }

    // TODO check duplication signature matches

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

void ModuleGenerator::visit(AST::Cast *cast) {
    cast->operand->accept(this);

    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Type *destination_type = generate_type(cast);
    llvm::Value *new_value = m_irBuilder->CreateBitCast(value, destination_type);
    m_llvmValues.push_back(new_value);
}

void ModuleGenerator::visit(AST::Assignment *expression) {
    expression->rhs->accept(this);
    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Value *ptr = m_scope->lookup(expression->lhs)->value;

    m_irBuilder->CreateStore(value, ptr);
}

void ModuleGenerator::visit(AST::Selector *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    expression->operand->accept(this);

    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->operand);
    if (!identifier) {
        throw Errors::InternalError(expression, "N/A");
    }

    SymbolTable::Symbol *symbol = m_scope->lookup(identifier);

    llvm::Value *instance = symbol->value;

    auto recordType = static_cast<Types::Record *>(expression->operand->type);
    uint64_t index = recordType->get_field_index(expression->name->value);

    std::vector<llvm::Value *> indexes;
    indexes.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(context, 32), 0));
    indexes.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(context, 32), index));

    llvm::Value *value = m_irBuilder->CreateInBoundsGEP(instance, indexes);
    m_llvmValues.push_back(m_irBuilder->CreateLoad(value));
}

void ModuleGenerator::visit(AST::Index *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    expression->operand->accept(this);

    AST::Identifier *identifier = dynamic_cast<AST::Identifier *>(expression->operand);
    if (!identifier) {
        throw Errors::InternalError(expression, "N/A");
    }

    SymbolTable::Symbol *symbol = m_scope->lookup(identifier);

    llvm::Value *instance = symbol->value;

    expression->index->accept(this);
    llvm::Value *index = m_llvmValues.back();
    m_llvmValues.pop_back();

    auto i32 = llvm::IntegerType::get(context, 32);
    auto i64 = llvm::IntegerType::get(context, 64);

    std::vector<llvm::Value *> indexes;
    indexes.push_back(llvm::ConstantInt::get(i32, 0));
    indexes.push_back(llvm::ConstantInt::get(i32, 1));

    llvm::Value *elements_value = m_irBuilder->CreateInBoundsGEP(instance, indexes, "elements");

    std::vector<llvm::Value *> element_index;
    element_index.push_back(llvm::ConstantInt::get(i32, 0));
    //element_index.push_back(m_irBuilder->CreateIntCast(index, i32, true));

    llvm::Value *value = m_irBuilder->CreateInBoundsGEP(elements_value, element_index, "element");

    //llvm::Value *value = m_irBuilder->CreateAlloca()
    m_llvmValues.push_back(m_irBuilder->CreateLoad(value));
}

void ModuleGenerator::visit(AST::Comma *expression) {
    throw Errors::InternalError(expression, "N/A");
}

void ModuleGenerator::visit(AST::While *expression) {
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

void ModuleGenerator::visit(AST::For *expression) {
    throw Errors::InternalError(expression, "Should not be in the lowered AST.");
}

void ModuleGenerator::visit(AST::If *expression) {
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

    llvm::Type *type = generate_type(expression);
    llvm::PHINode *phi = m_irBuilder->CreatePHI(type, 2, "iftmp");
    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    m_llvmValues.push_back(phi);

    debug("Ended if statement.");
}

void ModuleGenerator::visit(AST::Return *expression) {
    debug("Generating return statement.");

    expression->expression->accept(this);

    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Value *returnValue = m_irBuilder->CreateRet(value);
    m_llvmValues.push_back(returnValue);
}

void ModuleGenerator::visit(AST::Spawn *expression) {
    debug("Generating spawn statement.");
}

void ModuleGenerator::visit(AST::Parameter *parameter) {
    throw Errors::InternalError(parameter, "N/A");
}

void ModuleGenerator::visit(AST::VariableDefinition *definition) {
    SymbolTable::Symbol *symbol = m_scope->lookup(definition->name);

    debug("Defining a new variable: " + symbol->name);

    if (m_scope->is_root()) {
        llvm::Type *type = generate_type(definition);
        llvm::Constant *initialiser = m_type_generator->take_initialiser(definition);

        llvm::GlobalVariable *variable = new llvm::GlobalVariable(*m_module, type, false,
                                                                  llvm::GlobalValue::CommonLinkage,
                                                                  nullptr, definition->name->value);
        variable->setAlignment(4);
        variable->setVisibility(llvm::GlobalValue::DefaultVisibility);
        //variable->setInitializer(llvm::UndefValue::get(type));
        variable->setInitializer(initialiser);

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

        llvm::Type *type = generate_type(definition);

        llvm::IRBuilder<> tmp_ir(&function->getEntryBlock(), function->getEntryBlock().begin());
        llvm::AllocaInst *variable = tmp_ir.CreateAlloca(type, 0, definition->name->value);
        m_irBuilder->CreateStore(initialiser, variable);
        symbol->value = variable;
    }
}

void ModuleGenerator::visit(AST::FunctionDefinition *definition) {
    SymbolTable::Symbol *functionSymbol = m_scope->lookup(definition->name);

    Types::Method *method = static_cast<Types::Method *>(definition->type);

    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup(definition, method->mangled_name());

    std::string llvm_function_name = Mangler::mangle_method(functionSymbol->name, method);

    SymbolTable::Namespace *oldNamespace = m_scope;
    m_scope = symbol->nameSpace;

    llvm::Type *llvmType = generate_type(definition, method);

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(llvmType);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_function_name, m_module);

    // function->setGC("shadow-stack");

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    m_irBuilder->SetInsertPoint(basicBlock);

    int i = 0;
    for (auto &arg : function->args()) {
        std::string name2 = method->get_parameter_name(i);
        arg.setName(method->get_parameter_name(i));
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

void ModuleGenerator::visit(AST::TypeDefinition *definition) {
    std::string name = definition->name->value;

    if (definition->alias) {
        // ignore
        return;
    }

    llvm::LLVMContext &context = llvm::getGlobalContext();

    // create initialiser function
    llvm::Type *return_type = generate_type(definition, static_cast<Types::Constructor *>(definition->type)->create(definition));

    std::vector<llvm::Type *> element_types;
    for (auto field : definition->fields) {
        element_types.push_back(generate_type(field));
    }

    llvm::FunctionType *function_type = llvm::FunctionType::get(return_type, element_types, false);

    std::string function_name = name + "_new";

    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, function_name, m_module);
    function->addFnAttr(llvm::Attribute::AlwaysInline);

    int i = 0;
    for (auto &arg : function->args()) {
        arg.setName(definition->fields[i]->name->value);
        i++;
    }

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(context, "entry", function);
    m_irBuilder->SetInsertPoint(basicBlock);

    auto instance = m_irBuilder->CreateAlloca(return_type);

    llvm::Function::arg_iterator args = function->arg_begin();

    auto i32 = llvm::IntegerType::get(context, 32);
    auto index0 = llvm::ConstantInt::get(i32, 0);
    for (int i = 0; i < definition->fields.size(); i++) {
        auto index = llvm::ConstantInt::get(i32, i);

        std::vector<llvm::Value *> indexes;
        indexes.push_back(index0);
        indexes.push_back(index);

        auto ptr = m_irBuilder->CreateInBoundsGEP(instance, indexes);

        llvm::Argument *arg = &(*(args++));
        m_irBuilder->CreateStore(arg, ptr);
    }

    m_irBuilder->CreateRet(m_irBuilder->CreateLoad(instance));

    llvm::Function *mainFunction = m_module->getFunction("main");
    m_irBuilder->SetInsertPoint(&mainFunction->getEntryBlock());
}

void ModuleGenerator::visit(AST::DefinitionStatement *statement) {
    debug("Generating definition statement.");
    statement->definition->accept(this);
}

void ModuleGenerator::visit(AST::ExpressionStatement *statement) {
    debug("Generating expression statement.");
    statement->expression->accept(this);
}

void ModuleGenerator::visit(AST::ImportStatement *statement) {
    throw Errors::InternalError(statement, "N/A");
}

void ModuleGenerator::visit(AST::SourceFile *module) {
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
