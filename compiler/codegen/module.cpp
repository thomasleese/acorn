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
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>

#include "../ast/nodes.h"
#include "../builtins.h"
#include "../errors.h"
#include "../symbolTable.h"
#include "../types.h"
#include "types.h"

#include "module.h"

using namespace acorn;
using namespace acorn::codegen;

#define return_if_null(thing) if (thing == nullptr) return;

std::string codegen::mangle_method(std::string name, types::Method *type) {
    return "_A_" + name + "_" + type->mangled_name();
}

ModuleGenerator::ModuleGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout) :
        m_type_generator(new TypeGenerator()) {
    m_scope.push_back(scope);

    m_irBuilder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    m_mdBuilder = new llvm::MDBuilder(llvm::getGlobalContext());
    m_data_layout = data_layout;
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

llvm::Type *ModuleGenerator::generate_type(ast::Node *node, types::Type *type) {
    type->accept(m_type_generator);
    auto result = m_type_generator->take_type(node);
    if (result == nullptr) {
        push_error(m_type_generator->next_error());
    }
    return result;
}

llvm::Type *ModuleGenerator::generate_type(ast::Node *node) {
    return generate_type(node, node->type);
}

llvm::Function *ModuleGenerator::generate_function(ast::FunctionDefinition *definition) {
    std::map<types::ParameterConstructor *, types::Type *> params;
    return generate_function(definition, params);
}

llvm::Function *ModuleGenerator::generate_function(ast::FunctionDefinition *definition, std::map<types::ParameterConstructor *, types::Type *> type_parameters) {
    auto function_symbol = m_scope.back()->lookup(this, definition->name);
    auto method = static_cast<types::Method *>(definition->type);
    auto symbol = function_symbol->nameSpace->lookup_by_node(this, definition);

    std::string llvm_function_name = codegen::mangle_method(function_symbol->name, method);

    if (!type_parameters.empty()) {
        llvm_function_name += "_";
        for (auto entry : type_parameters) {
            auto t = entry.second;
            while (auto p = dynamic_cast<types::Parameter *>(t)) {
                t = m_type_generator->get_type_parameter(p);
                if (t == nullptr) {
                    return nullptr;
                }
            }
            llvm_function_name += t->mangled_name();
        }
    }

    m_scope.push_back(symbol->nameSpace);

    for (auto entry : type_parameters) {
        m_type_generator->push_type_parameter(entry.first, entry.second);
    }

    llvm::Type *llvmType = generate_type(definition, method);
    if (llvmType == nullptr) {
        return nullptr;
    }

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(llvmType);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_function_name, m_module);

    auto old_insert_point = m_irBuilder->saveIP();

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    m_irBuilder->SetInsertPoint(basicBlock);

    int i = 0;
    for (auto &arg : function->args()) {
        std::string arg_name = definition->parameters[i]->name->value;
        arg.setName(arg_name);

        llvm::Value *value = &arg;

        if (!definition->parameters[i]->inout) {
            auto alloca = m_irBuilder->CreateAlloca(arg.getType(), 0, arg_name);
            m_irBuilder->CreateStore(&arg, alloca);
            value = alloca;
        }

        auto arg_symbol = m_scope.back()->lookup(this, definition, arg_name);
        arg_symbol->value = value;

        i++;
    }

    definition->code->accept(this);

    auto nothing = m_irBuilder->getInt1(false);
    if (definition->code->statements.empty()) {
        m_irBuilder->CreateRet(nothing);
    } else {
        if (m_llvmValues.empty()) {
            m_irBuilder->CreateRet(nothing);
        } else {
            llvm::Value *value = m_llvmValues.back();
            m_llvmValues.pop_back();

            if (value == nullptr || !llvm::isa<llvm::ReturnInst>(value)) {
                m_irBuilder->CreateRet(nothing);
            }
        }
    }

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
        m_function_pass_manager->run(*function);
        function->dump();
        push_error(new errors::InternalError(definition, stream.str()));
    }

    m_scope.pop_back();

    for (auto entry : type_parameters) {
        m_type_generator->pop_type_parameter(entry.first);
    }

    m_irBuilder->restoreIP(old_insert_point);

    return function;
}

void ModuleGenerator::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void ModuleGenerator::visit(ast::Identifier *identifier) {
    debug("Finding named value: " + identifier->value);

    auto symbol = m_scope.back()->lookup(this, identifier);
    if (symbol == nullptr) {
        m_llvmValues.push_back(nullptr);
        return;
    }

    if (!symbol->value) {
        push_error(new errors::InternalError(identifier, "should not be nullptr"));
        m_llvmValues.push_back(nullptr);
        return;
    }

    if (identifier->has_parameters()) {
        push_error(new errors::InternalError(identifier, "should not have parameters"));
        m_llvmValues.push_back(nullptr);
        return;
    }

    llvm::Value *value = m_irBuilder->CreateLoad(symbol->value);
    m_llvmValues.push_back(value);
}

void ModuleGenerator::visit(ast::VariableDeclaration *node) {
    auto symbol = m_scope.back()->lookup(this, node->name());

    auto llvm_type = generate_type(node);
    return_if_null(llvm_type);

    auto old_insert_point = m_irBuilder->saveIP();

    if (m_scope.back()->is_root()) {
        auto llvm_initialiser = m_type_generator->take_initialiser(node);
        if (llvm_initialiser == nullptr) {
            push_error(m_type_generator->next_error());
            return;
        }

        auto variable = new llvm::GlobalVariable(*m_module, llvm_type, false,
                                                 llvm::GlobalValue::CommonLinkage,
                                                 llvm_initialiser, node->name()->value);
        variable->setAlignment(4);
        variable->setVisibility(llvm::GlobalValue::DefaultVisibility);

        symbol->value = variable;

        auto insert_function = m_module->getFunction("_init_variables_");
        m_irBuilder->SetInsertPoint(&insert_function->getEntryBlock());
    } else {
        auto insert_function = m_irBuilder->GetInsertBlock()->getParent();
        m_irBuilder->SetInsertPoint(&insert_function->getEntryBlock());

        symbol->value = m_irBuilder->CreateAlloca(llvm_type, 0, node->name()->value);
    }

    m_irBuilder->restoreIP(old_insert_point);

    m_llvmValues.push_back(symbol->value);
}

void ModuleGenerator::visit(ast::BooleanLiteral *boolean) {
    push_error(new errors::InternalError(boolean, "N/A"));
    m_llvmValues.push_back(nullptr);
}

void ModuleGenerator::visit(ast::IntegerLiteral *expression) {
    debug("Making integer literal: " + expression->value);

    llvm::Type *type = generate_type(expression);

    uint64_t integer;
    std::stringstream ss;
    ss << expression->value;
    ss >> integer;

    llvm::Constant *value = llvm::ConstantInt::get(type, integer, true);
    m_llvmValues.push_back(value);
}

void ModuleGenerator::visit(ast::FloatLiteral *expression) {
    debug("Making float literal: " + expression->value);

    llvm::Type *type = generate_type(expression);

    double floatValue;
    std::stringstream ss;
    ss << expression->value;
    ss >> floatValue;

    llvm::Constant *value = llvm::ConstantFP::get(type, floatValue);
    m_llvmValues.push_back(value);
}

void ModuleGenerator::visit(ast::ImaginaryLiteral *imaginary) {
    push_error(new errors::InternalError(imaginary, "N/A"));
    m_llvmValues.push_back(nullptr);
}

void ModuleGenerator::visit(ast::StringLiteral *expression) {
    push_error(new errors::InternalError(expression, "N/A"));
    m_llvmValues.push_back(nullptr);
}

void ModuleGenerator::visit(ast::SequenceLiteral *sequence) {
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

void ModuleGenerator::visit(ast::MappingLiteral *mapping) {
    push_error(new errors::InternalError(mapping, "N/A"));
    m_llvmValues.push_back(nullptr);
}

void ModuleGenerator::visit(ast::RecordLiteral *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    //auto record = dynamic_cast<types::Record *>(expression->type);

    llvm::Type *llvm_type = generate_type(expression);

    auto instance = m_irBuilder->CreateAlloca(llvm_type);

    auto i32 = llvm::IntegerType::get(context, 32);
    auto index0 = llvm::ConstantInt::get(i32, 0);
    for (int i = 0; i < expression->field_names.size(); i++) {
        auto index = llvm::ConstantInt::get(i32, i);

        expression->field_values[i]->accept(this);

        auto value = m_llvmValues.back();
        m_llvmValues.pop_back();

        std::vector<llvm::Value *> indexes;
        indexes.push_back(index0);
        indexes.push_back(index);

        auto ptr = m_irBuilder->CreateInBoundsGEP(instance, indexes);
        m_irBuilder->CreateStore(value, ptr);
    }

    m_llvmValues.push_back(m_irBuilder->CreateLoad(instance));
}

void ModuleGenerator::visit(ast::Call *expression) {
    ast::Identifier *identifier = dynamic_cast<ast::Identifier *>(expression->operand);
    if (identifier) {
        debug("Generating a call to: " + identifier->value);

        auto symbol = m_scope.back()->lookup(this, identifier);
        types::Function *functionType = dynamic_cast<types::Function *>(symbol->type);

        std::vector<types::Type *> argument_types;
        for (auto arg : expression->arguments) {
            argument_types.push_back(arg->type);
        }

        auto method = functionType->find_method(expression, argument_types);
        assert(method);

        std::string method_name = codegen::mangle_method(symbol->name, method);
        auto method_symbol = symbol->nameSpace->lookup(this, expression, method->mangled_name());
        auto definition = static_cast<ast::FunctionDefinition *>(method_symbol->node);

        if (method->is_generic()) {
            std::map<types::ParameterConstructor *, types::Type *> type_parameters = expression->inferred_type_parameters;

            method_name += "_";
            for (auto entry : type_parameters) {
                auto t = entry.second;
                while (auto p = dynamic_cast<types::Parameter *>(t)) {
                    t = m_type_generator->get_type_parameter(p);
                    if (t == nullptr) {
                        return;
                    }
                }
                method_name += t->mangled_name();
            }

            if (m_module->getFunction(method_name) == nullptr) {
                if (definition == nullptr) {
                    for (auto entry : type_parameters) {
                        m_type_generator->push_type_parameter(entry.first, entry.second);
                    }

                    builtins::generate_function(symbol, method_symbol, method, method_name, m_module, m_irBuilder, m_type_generator);

                    for (auto entry : type_parameters) {
                        m_type_generator->pop_type_parameter(entry.first);
                    }
                } else {
                    generate_function(definition, type_parameters);
                }
            }
        }

        auto function = m_module->getFunction(method_name);
        if (function == nullptr) {
            push_error(new errors::InternalError(expression, "No LLVM function created: " + method_name + " (" + method->name() + ")!"));
            return;
        }

        std::vector<llvm::Value *> arguments;
        int i = 0;
        for (auto argument : expression->arguments) {
            argument->accept(this);

            auto value = m_llvmValues.back();
            m_llvmValues.pop_back();

            if (method->is_parameter_inout(method->parameter_types()[i])) {
                llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(value);
                assert(load);

                value = load->getPointerOperand();
            }

            arguments.push_back(value);
            i++;
        }

        auto value = m_irBuilder->CreateCall(function, arguments);
        m_llvmValues.push_back(value);

        debug("Ending call to: " + identifier->value);
    } else {
        push_error(new errors::InternalError(expression, "Not an identifier."));
    }
}

void ModuleGenerator::visit(ast::CCall *ccall) {
    std::vector<llvm::Type *> parameters;
    llvm::Type *returnType = generate_type(ccall);

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

void ModuleGenerator::visit(ast::Cast *cast) {
    cast->operand->accept(this);

    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Type *destination_type = generate_type(cast);
    llvm::Value *new_value = m_irBuilder->CreateBitCast(value, destination_type);
    m_llvmValues.push_back(new_value);
}

void ModuleGenerator::visit(ast::Assignment *expression) {
    expression->rhs->accept(this);

    auto rhs_value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Value *rhs_variable_pointer = nullptr;
    if (auto load = dynamic_cast<llvm::LoadInst *>(rhs_value)) {
        rhs_variable_pointer = load->getPointerOperand();
    } else if (auto alloca = dynamic_cast<llvm::AllocaInst *>(rhs_value)) {
        rhs_variable_pointer = alloca;
    } else if (auto gv = dynamic_cast<llvm::GlobalVariable *>(rhs_value)) {
        rhs_variable_pointer = gv;
    }

    expression->lhs->accept(this);

    auto last_value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Value *variable_pointer = nullptr;
    if (auto load = dynamic_cast<llvm::LoadInst *>(last_value)) {
        variable_pointer = load->getPointerOperand();
    } else if (auto alloca = dynamic_cast<llvm::AllocaInst *>(last_value)) {
        variable_pointer = alloca;
    } else if (auto gv = dynamic_cast<llvm::GlobalVariable *>(last_value)) {
        variable_pointer = gv;
    }

    assert(variable_pointer);

    auto lhs_union_type = dynamic_cast<types::Union *>(expression->lhs->type);
    auto rhs_union_type = dynamic_cast<types::Union *>(expression->rhs->type);

    if (lhs_union_type && !rhs_union_type) {
        bool ok;
        uint8_t index = lhs_union_type->type_index(expression->rhs->type, &ok);
        assert(ok);  // type checker should catch this

        std::vector<llvm::Value *> indexes;
        indexes.push_back(m_irBuilder->getInt32(0));
        indexes.push_back(m_irBuilder->getInt32(0));

        auto index_gep = m_irBuilder->CreateInBoundsGEP(variable_pointer, indexes);
        m_irBuilder->CreateStore(m_irBuilder->getInt8(index), index_gep);

        indexes.clear();
        indexes.push_back(m_irBuilder->getInt32(0));
        indexes.push_back(m_irBuilder->getInt32(index + 1));

        auto holder_gep = m_irBuilder->CreateInBoundsGEP(variable_pointer, indexes);
        m_irBuilder->CreateStore(rhs_value, holder_gep);

        m_llvmValues.push_back(variable_pointer);
    } else if (rhs_union_type && !lhs_union_type) {
        bool ok;
        uint8_t index_we_want = rhs_union_type->type_index(expression->lhs->type, &ok);
        assert(ok);  // type checker should catch this

        std::vector<llvm::Value *> indexes;
        indexes.push_back(m_irBuilder->getInt32(0));
        indexes.push_back(m_irBuilder->getInt32(0));

        assert(rhs_variable_pointer);

        auto index_we_have_gep = m_irBuilder->CreateInBoundsGEP(rhs_variable_pointer, indexes, "union_index_ptr");
        auto index_we_have = m_irBuilder->CreateLoad(index_we_have_gep, "union_index");

        indexes.clear();
        indexes.push_back(m_irBuilder->getInt32(0));
        indexes.push_back(m_irBuilder->getInt32(1 + index_we_want));

        auto holder_gep = m_irBuilder->CreateInBoundsGEP(rhs_variable_pointer, indexes, "union_index_value_ptr");
        m_irBuilder->CreateStore(m_irBuilder->CreateLoad(holder_gep, "union_index"), variable_pointer);

        auto icmp = m_irBuilder->CreateICmpEQ(m_irBuilder->getInt8(index_we_want), index_we_have, "check_union_type");
        m_llvmValues.push_back(icmp);
    } else {
        m_irBuilder->CreateStore(rhs_value, variable_pointer);
        m_llvmValues.push_back(variable_pointer);
    }
}

void ModuleGenerator::visit(ast::Selector *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    expression->operand->accept(this);

    ast::Identifier *identifier = dynamic_cast<ast::Identifier *>(expression->operand);
    if (!identifier) {
        push_error(new errors::InternalError(expression, "N/A"));
    }

    auto symbol = m_scope.back()->lookup(this, identifier);

    llvm::Value *instance = symbol->value;

    types::Record *recordType = static_cast<types::Record *>(expression->operand->type);

    uint64_t index = recordType->get_field_index(expression->name->value);

    std::vector<llvm::Value *> indexes;
    indexes.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(context, 32), 0));
    indexes.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(context, 32), index));

    llvm::Value *value = m_irBuilder->CreateInBoundsGEP(instance, indexes);
    m_llvmValues.push_back(m_irBuilder->CreateLoad(value));
}

void ModuleGenerator::visit(ast::Comma *expression) {
    push_error(new errors::InternalError(expression, "N/A"));
}

void ModuleGenerator::visit(ast::While *expression) {
    push_error(new errors::InternalError(expression, "N/A"));
}

void ModuleGenerator::visit(ast::For *expression) {
    push_error(new errors::InternalError(expression, "Should not be in the lowered AST."));
}

void ModuleGenerator::visit(ast::If *expression) {
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

    llvm::Value *else_value = nullptr;
    if (expression->falseCode) {
        debug("Generating false code...");
        expression->falseCode->accept(this);
        else_value = m_llvmValues.back();
        m_llvmValues.pop_back();
    } else {
        else_value = m_irBuilder->getInt1(false);
    }

    m_irBuilder->CreateBr(merge_bb);

    else_bb = m_irBuilder->GetInsertBlock();

    function->getBasicBlockList().push_back(merge_bb);
    m_irBuilder->SetInsertPoint(merge_bb);

    llvm::Type *type = generate_type(expression);
    llvm::PHINode *phi = m_irBuilder->CreatePHI(type, 2, "iftmp");

    type->dump();
    then_value->dump();
    else_value->dump();

    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    m_llvmValues.push_back(phi);

    debug("Ended if statement.");
}

void ModuleGenerator::visit(ast::Return *expression) {
    debug("Generating return statement.");

    expression->expression->accept(this);

    llvm::Value *value = m_llvmValues.back();
    m_llvmValues.pop_back();

    llvm::Value *returnValue = m_irBuilder->CreateRet(value);
    m_llvmValues.push_back(returnValue);
}

void ModuleGenerator::visit(ast::Spawn *expression) {
    debug("Generating spawn statement.");
}

void ModuleGenerator::visit(ast::Sizeof *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    llvm::Type *type = generate_type(expression);
    uint64_t size = m_data_layout->getTypeStoreSize(type);

    auto i64 = llvm::IntegerType::getInt64Ty(context);
    m_llvmValues.push_back(llvm::ConstantInt::get(i64, size, true));
}

void ModuleGenerator::visit(ast::Strideof *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    llvm::Type *type = generate_type(expression);
    uint64_t size = m_data_layout->getTypeAllocSize(type);

    auto i64 = llvm::IntegerType::getInt64Ty(context);
    m_llvmValues.push_back(llvm::ConstantInt::get(i64, size, true));
}

void ModuleGenerator::visit(ast::Parameter *parameter) {
    push_error(new errors::InternalError(parameter, "N/A"));
}

void ModuleGenerator::visit(ast::VariableDefinition *definition) {
    definition->assignment->accept(this);
}

void ModuleGenerator::visit(ast::FunctionDefinition *definition) {
    if (definition->name->parameters.empty()) {
        generate_function(definition);
    }
}

void ModuleGenerator::visit(ast::TypeDefinition *definition) {
    if (definition->alias) {
        auto new_symbol = m_scope.back()->lookup(this, definition->name);
        auto old_symbol = m_scope.back()->lookup(this, definition->alias);
        new_symbol->value = old_symbol->value;
    }
}

void ModuleGenerator::visit(ast::DefinitionStatement *statement) {
    debug("Generating definition statement.");
    statement->definition->accept(this);
}

void ModuleGenerator::visit(ast::ExpressionStatement *statement) {
    debug("Generating expression statement.");
    statement->expression->accept(this);
}

void ModuleGenerator::visit(ast::ImportStatement *statement) {
    push_error(new errors::InternalError(statement, "N/A"));
}

void ModuleGenerator::visit(ast::SourceFile *module) {
    m_module = new llvm::Module(module->name, llvm::getGlobalContext());

    m_function_pass_manager = new llvm::legacy::FunctionPassManager(m_module);
    //m_function_pass_manager->add(llvm::createFunctionInliningPass());
    //m_function_pass_manager->add(llvm::createBasicAliasAnalysisPass());
    //m_function_pass_manager->add(llvm::createInstructionCombiningPass());
    //m_function_pass_manager->add(llvm::createReassociatePass());
    //m_function_pass_manager->add(llvm::createGVNPass());
    //m_function_pass_manager->add(llvm::createCFGSimplificationPass());
    //m_function_pass_manager->add(llvm::createInstructionCombiningPass());
    m_function_pass_manager->doFinalization();

    builtins::fill_llvm_module(m_scope.back(), m_module, m_irBuilder);

    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "_init_variables_", m_module);
    llvm::BasicBlock *bb1 = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);

    fType = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), false);
    llvm::Function *mainFunction = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "main", m_module);
    llvm::BasicBlock *main_bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction);

    m_irBuilder->SetInsertPoint(main_bb);
    m_irBuilder->CreateCall(function);

    module->code->accept(this);

    m_irBuilder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), 0));

    m_irBuilder->SetInsertPoint(bb1);
    m_irBuilder->CreateRetVoid();

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyModule(*m_module, &stream)) {
        m_module->dump();
        push_error(new errors::InternalError(module, stream.str()));
    }
}
