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
#include "../errors.h"
#include "../symbolTable.h"
#include "../typing/types.h"
#include "builtins.h"
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
    std::map<types::ParameterType *, types::Type *> params;
    return generate_function(definition, params);
}

llvm::Function *ModuleGenerator::generate_function(ast::FunctionDefinition *definition, std::map<types::ParameterType *, types::Type *> type_parameters) {
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
            auto value = pop_value();

            if (value == nullptr || !llvm::isa<llvm::ReturnInst>(value)) {
                m_irBuilder->CreateRet(nothing);
            }
        }
    }

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
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

void ModuleGenerator::push_value(llvm::Value *value) {
    m_llvmValues.push_back(value);
}

llvm::Value *ModuleGenerator::pop_value() {
    assert(!m_llvmValues.empty());

    auto value = m_llvmValues.back();
    m_llvmValues.pop_back();
    return value;
}

void ModuleGenerator::visit(ast::CodeBlock *block) {
    llvm::Value *last_value = nullptr;

    for (auto statement : block->statements) {
        statement->accept(this);
        last_value = pop_value();
    }

    push_value(last_value);
}

void ModuleGenerator::visit(ast::Identifier *identifier) {
    auto symbol = m_scope.back()->lookup(this, identifier);
    if (symbol == nullptr) {
        push_value(nullptr);
        return;
    }

    if (!symbol->value) {
        push_error(new errors::InternalError(identifier, "should not be nullptr"));
        push_value(nullptr);
        return;
    }

    if (identifier->has_parameters()) {
        push_error(new errors::InternalError(identifier, "should not have parameters"));
        push_value(nullptr);
        return;
    }

    llvm::Value *value = m_irBuilder->CreateLoad(symbol->value);
    push_value(value);
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

    push_value(symbol->value);
}

void ModuleGenerator::visit(ast::IntegerLiteral *expression) {
    auto type = generate_type(expression);

    uint64_t integer;
    std::stringstream ss;
    ss << expression->value;
    ss >> integer;

    push_value(llvm::ConstantInt::get(type, integer, true));
}

void ModuleGenerator::visit(ast::FloatLiteral *expression) {
    auto type = generate_type(expression);

    double floatValue;
    std::stringstream ss;
    ss << expression->value;
    ss >> floatValue;

    push_value(llvm::ConstantFP::get(type, floatValue));
}

void ModuleGenerator::visit(ast::ImaginaryLiteral *imaginary) {
    push_error(new errors::InternalError(imaginary, "N/A"));
    push_value(nullptr);
}

void ModuleGenerator::visit(ast::StringLiteral *expression) {
    push_error(new errors::InternalError(expression, "N/A"));
    push_value(nullptr);
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

    push_value(m_irBuilder->CreateLoad(instance));
}

void ModuleGenerator::visit(ast::MappingLiteral *mapping) {
    push_error(new errors::InternalError(mapping, "N/A"));
    push_value(nullptr);
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

        auto value = pop_value();

        std::vector<llvm::Value *> indexes;
        indexes.push_back(index0);
        indexes.push_back(index);

        auto ptr = m_irBuilder->CreateInBoundsGEP(instance, indexes);
        m_irBuilder->CreateStore(value, ptr);
    }

    push_value(m_irBuilder->CreateLoad(instance));
}

void ModuleGenerator::visit(ast::TupleLiteral *expression) {
    auto &context = llvm::getGlobalContext();

    auto llvm_type = generate_type(expression);

    auto instance = m_irBuilder->CreateAlloca(llvm_type);

    auto i32 = llvm::IntegerType::get(context, 32);
    auto index0 = llvm::ConstantInt::get(i32, 0);

    auto elements = expression->elements();
    for (int i = 0; i < elements.size(); i++) {
        elements[i]->accept(this);
        auto value = pop_value();

        std::vector<llvm::Value *> indexes;
        indexes.push_back(index0);
        indexes.push_back(m_irBuilder->getInt32(i));

        auto ptr = m_irBuilder->CreateInBoundsGEP(instance, indexes);
        m_irBuilder->CreateStore(value, ptr);
    }

    push_value(m_irBuilder->CreateLoad(instance));
}

void ModuleGenerator::visit(ast::Call *expression) {
    ast::Identifier *identifier = dynamic_cast<ast::Identifier *>(expression->operand);
    if (identifier) {
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
            std::map<types::ParameterType *, types::Type *> type_parameters = expression->inferred_type_parameters;

            method_name += "_";
            for (auto entry : type_parameters) {
                auto t = entry.second;
                while (auto p = dynamic_cast<types::Parameter *>(t)) {
                    t = m_type_generator->get_type_parameter(p);
                    if (t == nullptr) {
                        push_value(nullptr);
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

                    m_builtin_generator->generate_function(symbol, method_symbol, method_name);
                    //builtins::generate_function(symbol, method_symbol, method, method_name, m_module, m_irBuilder, m_type_generator);

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
            push_value(nullptr);
            return;
        }

        std::vector<llvm::Value *> arguments;
        int i = 0;
        for (auto argument : expression->arguments) {
            argument->accept(this);

            auto value = pop_value();

            if (method->is_parameter_inout(method->parameter_types()[i])) {
                auto load = llvm::dyn_cast<llvm::LoadInst>(value);
                assert(load);

                value = load->getPointerOperand();
            }

            arguments.push_back(value);
            i++;
        }

        auto value = m_irBuilder->CreateCall(function, arguments);
        push_value(value);
    } else {
        push_error(new errors::InternalError(expression, "Not an identifier."));
        push_value(nullptr);
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

        auto argValue = pop_value();
        arguments.push_back(argValue);
    }

    llvm::Value *value = m_irBuilder->CreateCall(function, arguments);
    push_value(value);
}

void ModuleGenerator::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    auto value = pop_value();

    llvm::Type *destination_type = generate_type(cast);
    llvm::Value *new_value = m_irBuilder->CreateBitCast(value, destination_type);
    push_value(new_value);
}

void ModuleGenerator::visit(ast::Assignment *expression) {
    expression->rhs->accept(this);
    auto rhs_value = pop_value();
    return_if_null(rhs_value);

    llvm::Value *rhs_variable_pointer = nullptr;
    if (auto load = dynamic_cast<llvm::LoadInst *>(rhs_value)) {
        rhs_variable_pointer = load->getPointerOperand();
    } else if (auto alloca = dynamic_cast<llvm::AllocaInst *>(rhs_value)) {
        rhs_variable_pointer = alloca;
    } else if (auto gv = dynamic_cast<llvm::GlobalVariable *>(rhs_value)) {
        rhs_variable_pointer = gv;
    }

    expression->lhs->accept(this);
    auto lhs_value = pop_value();

    llvm::Value *variable_pointer = nullptr;
    if (auto load = dynamic_cast<llvm::LoadInst *>(lhs_value)) {
        variable_pointer = load->getPointerOperand();
    } else if (auto alloca = dynamic_cast<llvm::AllocaInst *>(lhs_value)) {
        variable_pointer = alloca;
    } else if (auto gv = dynamic_cast<llvm::GlobalVariable *>(lhs_value)) {
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

        push_value(variable_pointer);
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
        push_value(icmp);
    } else {
        m_irBuilder->CreateStore(rhs_value, variable_pointer);
        push_value(variable_pointer);
    }
}

void ModuleGenerator::visit(ast::Selector *expression) {
    llvm::LLVMContext &context = llvm::getGlobalContext();

    expression->operand->accept(this);
    pop_value();

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
    push_value(m_irBuilder->CreateLoad(value));
}

void ModuleGenerator::visit(ast::While *expression) {
    auto &context = llvm::getGlobalContext();

    auto function = m_irBuilder->GetInsertBlock()->getParent();

    auto entry_bb = llvm::BasicBlock::Create(context, "while_entry", function);
    m_irBuilder->CreateBr(entry_bb);

    m_irBuilder->SetInsertPoint(entry_bb);
    expression->condition()->accept(this);
    auto condition = m_irBuilder->CreateICmpEQ(pop_value(), m_irBuilder->getInt1(true), "while_cond");

    auto loop_bb = llvm::BasicBlock::Create(context, "while_loop", function);
    auto else_bb = llvm::BasicBlock::Create(context, "while_else", function);

    m_irBuilder->CreateCondBr(condition, loop_bb, else_bb);
    m_irBuilder->SetInsertPoint(loop_bb);

    expression->code()->accept(this);
    auto then_value = pop_value();
    push_value(then_value);
    m_irBuilder->CreateBr(entry_bb);

    m_irBuilder->SetInsertPoint(else_bb);
}

void ModuleGenerator::visit(ast::If *expression) {
    expression->condition->accept(this);
    auto condition = pop_value();

    llvm::LLVMContext &context = llvm::getGlobalContext();

    condition = m_irBuilder->CreateICmpEQ(condition, llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1), "ifcond");

    llvm::Function *function = m_irBuilder->GetInsertBlock()->getParent();

    llvm::BasicBlock *then_bb = llvm::BasicBlock::Create(context, "then", function);
    llvm::BasicBlock *else_bb = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock *merge_bb = llvm::BasicBlock::Create(context, "ifcont");

    m_irBuilder->CreateCondBr(condition, then_bb, else_bb);
    m_irBuilder->SetInsertPoint(then_bb);

    expression->trueCode->accept(this);
    auto then_value = pop_value();

    m_irBuilder->CreateBr(merge_bb);

    then_bb = m_irBuilder->GetInsertBlock();

    function->getBasicBlockList().push_back(else_bb);
    m_irBuilder->SetInsertPoint(else_bb);

    llvm::Value *else_value = nullptr;
    if (expression->falseCode) {
        expression->falseCode->accept(this);
        else_value = pop_value();
    } else {
        else_value = m_irBuilder->getInt1(false);
    }

    m_irBuilder->CreateBr(merge_bb);

    else_bb = m_irBuilder->GetInsertBlock();

    function->getBasicBlockList().push_back(merge_bb);
    m_irBuilder->SetInsertPoint(merge_bb);

    llvm::Type *type = generate_type(expression);
    llvm::PHINode *phi = m_irBuilder->CreatePHI(type, 2, "iftmp");

    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    push_value(phi);
}

void ModuleGenerator::visit(ast::Return *expression) {
    expression->expression->accept(this);
    auto value = pop_value();

    push_value(m_irBuilder->CreateRet(value));
}

void ModuleGenerator::visit(ast::Spawn *expression) {
    push_error(new errors::InternalError(expression, "N/A"));
    push_value(nullptr);
}

void ModuleGenerator::visit(ast::Sizeof *expression) {
    auto type = generate_type(expression);
    uint64_t size = m_data_layout->getTypeStoreSize(type);
    push_value(m_irBuilder->getInt64(size));
}

void ModuleGenerator::visit(ast::Strideof *expression) {
    auto type = generate_type(expression);
    uint64_t size = m_data_layout->getTypeAllocSize(type);
    push_value(m_irBuilder->getInt64(size));
}

void ModuleGenerator::visit(ast::Parameter *parameter) {
    push_error(new errors::InternalError(parameter, "N/A"));
}

void ModuleGenerator::visit(ast::VariableDefinition *definition) {
    definition->assignment->accept(this);
    pop_value();

    push_value(nullptr);
}

void ModuleGenerator::visit(ast::FunctionDefinition *definition) {
    if (definition->name->parameters.empty()) {
        generate_function(definition);
    }

    push_value(nullptr);
}

void ModuleGenerator::visit(ast::TypeDefinition *definition) {
    if (definition->alias) {
        auto new_symbol = m_scope.back()->lookup(this, definition->name);
        auto old_symbol = m_scope.back()->lookup(this, definition->alias);
        new_symbol->value = old_symbol->value;
    }

    push_value(nullptr);
}

void ModuleGenerator::visit(ast::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void ModuleGenerator::visit(ast::ExpressionStatement *statement) {
    statement->expression->accept(this);
    // we don't pop a value here because we should end up just pushing it again
}

void ModuleGenerator::visit(ast::ImportStatement *statement) {
    push_error(new errors::InternalError(statement, "N/A"));
}

void ModuleGenerator::visit(ast::SourceFile *module) {
    m_module = new llvm::Module(module->name, llvm::getGlobalContext());

    m_builtin_generator = new BuiltinGenerator(m_module, m_irBuilder, m_type_generator);
    m_builtin_generator->generate(m_scope.back());

    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "_init_variables_", m_module);
    llvm::BasicBlock *bb1 = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);

    fType = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), false);
    llvm::Function *mainFunction = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "main", m_module);
    llvm::BasicBlock *main_bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction);

    m_irBuilder->SetInsertPoint(main_bb);
    m_irBuilder->CreateCall(function);

    module->code->accept(this);
    assert(m_llvmValues.size() == 1);

    m_irBuilder->CreateRet(m_irBuilder->getInt32(0));

    m_irBuilder->SetInsertPoint(bb1);
    m_irBuilder->CreateRetVoid();

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyModule(*m_module, &stream)) {
        m_module->dump();
        push_error(new errors::InternalError(module, stream.str()));
    }
}
