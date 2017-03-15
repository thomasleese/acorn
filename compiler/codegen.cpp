//
// Created by Thomas Leese on 12/01/2017.
//

#include <iostream>
#include <sstream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>

#include "ast.h"
#include "diagnostics.h"
#include "symboltable.h"
#include "types.h"

#include "codegen.h"

using namespace acorn;
using namespace acorn::codegen;
using namespace acorn::diagnostics;

#define return_if_null(thing) if (thing == nullptr) return;

std::string codegen::mangle_method(std::string name, types::Method *type) {
    return "_A_" + name + "_" + type->mangled_name();
}

void ValueFollower::push_value(llvm::Value *value) {
    m_values.push_back(value);
}

llvm::Value *ValueFollower::pop_value() {
    assert(!m_units.empty());
    auto value = m_values.back();
    m_values.pop_back();
    return value;
}

llvm::Value *ValueFollower::value() const {
    return m_values.back();
}

llvm::Type *CodeGenerator::take_type(ast::Expression *expression)
{
    if (m_type_stack.size() >= 1) {
        llvm::Type *result = m_type_stack.back();
        m_type_stack.pop_back();

        if (expression && result == nullptr) {
            report(InternalError(expression, "Invalid LLVM type generated. (" + expression->type_name() + ")"));
            return nullptr;
        }

        return result;
    } else {
        report(InternalError(expression, "No LLVM type generated."));
        return nullptr;
    }
}

llvm::Constant *CodeGenerator::take_initialiser(ast::Node *node) {
    if (m_initialiser_stack.size() >= 1) {
        llvm::Constant *result = m_initialiser_stack.back();
        m_initialiser_stack.pop_back();

        if (node && result == nullptr) {
            report(InternalError(node, "Invalid LLVM initialiser generated."));
            return nullptr;
        }

        return result;
    } else {
        report(InternalError(node, "No LLVM initialiser generated."));
        return nullptr;
    }
}

void CodeGenerator::push_type_parameter(types::ParameterType *key, types::Type *value) {
    m_type_parameters[key] = value;
}

void CodeGenerator::pop_type_parameter(types::ParameterType *key) {
    m_type_parameters.erase(key);
}

types::Type *CodeGenerator::get_type_parameter(types::ParameterType *key) {
    return m_type_parameters[key];
}

types::Type *CodeGenerator::get_type_parameter(types::Parameter *key) {
    return get_type_parameter(key->type());
}

void CodeGenerator::visit_constructor(types::TypeType *type) {
    llvm::Type *llvm_type = llvm::Type::getInt1Ty(m_context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void CodeGenerator::visit(types::ParameterType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::VoidType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::BooleanType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::IntegerType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::UnsignedIntegerType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::FloatType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::UnsafePointerType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::FunctionType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::MethodType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::RecordType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::TupleType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::AliasType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::TypeDescriptionType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(types::Parameter *type) {
    auto it = m_type_parameters.find(type->type());
    if (it == m_type_parameters.end()) {
        m_type_stack.push_back(nullptr);
        m_initialiser_stack.push_back(nullptr);
    } else {
        it->second->accept(this);
    }
}

void CodeGenerator::visit(types::Void *type) {
    llvm::Type *llvm_type = llvm::Type::getInt1Ty(m_context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void CodeGenerator::visit(types::Boolean *type) {
    llvm::Type *llvm_type = llvm::Type::getInt1Ty(m_context);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void CodeGenerator::visit(types::Integer *type) {
    const unsigned int size = type->size();
    llvm::Type *llvm_type = llvm::IntegerType::getIntNTy(m_context, size);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void CodeGenerator::visit(types::UnsignedInteger *type) {
    const unsigned int size = type->size();
    llvm::Type *llvm_type = llvm::IntegerType::getIntNTy(m_context, size);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantInt::get(llvm_type, 0));
}

void CodeGenerator::visit(types::Float *type) {
    const unsigned int size = type->size();
    llvm::Type *llvm_type = nullptr;

    switch (size) {
        case 64:
            llvm_type = llvm::Type::getDoubleTy(m_context);
            break;
        case 32:
            llvm_type = llvm::Type::getFloatTy(m_context);
            break;
        case 16:
            llvm_type = llvm::Type::getHalfTy(m_context);
            break;
        default:
            break;
    }

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(llvm::ConstantFP::get(llvm_type, 0));
}

void CodeGenerator::visit(types::UnsafePointer *type) {
    type->element_type()->accept(this);
    llvm::Type *element_type = take_type(nullptr);

    if (element_type) {
        llvm::PointerType *pointer_type = llvm::PointerType::getUnqual(element_type);
        m_type_stack.push_back(pointer_type);
        m_initialiser_stack.push_back(llvm::ConstantPointerNull::get(pointer_type));
    } else {
        m_type_stack.push_back(nullptr);
        m_initialiser_stack.push_back(nullptr);
    }
}

void CodeGenerator::visit(types::Record *type) {
    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (auto field_type : type->field_types()) {
        field_type->accept(this);

        llvm::Type *llvm_field_type = take_type(nullptr);
        llvm::Constant *llvm_field_initialiser = take_initialiser(nullptr);

        if (llvm_field_type == nullptr || llvm_field_initialiser == nullptr) {
            m_type_stack.push_back(nullptr);
            m_initialiser_stack.push_back(nullptr);
            return;
        }

        llvm_types.push_back(llvm_field_type);
        llvm_initialisers.push_back(llvm_field_initialiser);
    }

    auto struct_type = llvm::StructType::get(m_context, llvm_types);
    auto struct_initialiser = llvm::ConstantStruct::get(struct_type, llvm_initialisers);

    m_type_stack.push_back(struct_type);
    m_initialiser_stack.push_back(struct_initialiser);
}

void CodeGenerator::visit(types::Tuple *type) {
    visit(static_cast<types::Record *>(type));
}

void CodeGenerator::visit(types::Method *type) {
    type->return_type()->accept(this);
    llvm::Type *llvm_return_type = take_type(nullptr);

    if (!llvm_return_type) {
        m_type_stack.push_back(nullptr);
        m_initialiser_stack.push_back(nullptr);
        return;
    }

    std::vector<llvm::Type *> llvm_parameter_types;
    for (auto parameter_type : type->parameter_types()) {
        parameter_type->accept(this);
        auto llvm_parameter_type = take_type(nullptr);

        if (!llvm_parameter_type) {
            m_type_stack.push_back(nullptr);
            m_initialiser_stack.push_back(nullptr);
            return;
        }

        if (type->is_parameter_inout(parameter_type)) {
            llvm_parameter_type = llvm::PointerType::getUnqual(llvm_parameter_type);
        }

        llvm_parameter_types.push_back(llvm_parameter_type);
    }

    auto llvm_type = llvm::FunctionType::get(llvm_return_type,
                                             llvm_parameter_types,
                                             false);
    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(nullptr);
}

void CodeGenerator::visit(types::Function *type) {
    std::vector<llvm::Type *> function_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (int i = 0; i < type->no_methods(); i++) {
        auto method = type->get_method(i);
        method->accept(this);
        auto function_type = take_type(nullptr);
        auto pointer_type = llvm::PointerType::getUnqual(function_type);
        function_types.push_back(pointer_type);
        llvm_initialisers.push_back(llvm::ConstantPointerNull::get(pointer_type));
    }

    auto llvm_type = llvm::StructType::get(m_context, function_types);
    auto struct_initialiser = llvm::ConstantStruct::get(llvm_type, llvm_initialisers);

    m_type_stack.push_back(llvm_type);
    m_initialiser_stack.push_back(struct_initialiser);
}

CodeGenerator::CodeGenerator(symboltable::Namespace *scope, llvm::LLVMContext &context, llvm::DataLayout *data_layout) : m_context(context)
{
    push_scope(scope);

    m_ir_builder = new llvm::IRBuilder<>(context);
    m_md_builder = new llvm::MDBuilder(context);
    m_data_layout = data_layout;
}

CodeGenerator::~CodeGenerator() {
    delete m_ir_builder;
    delete m_md_builder;
}

llvm::Module *CodeGenerator::module() const {
    return m_module;
}

llvm::Type *CodeGenerator::generate_type(ast::Expression *expression, types::Type *type) {
    type->accept(this);
    return take_type(expression);
}

llvm::Type *CodeGenerator::generate_type(ast::Expression *expression) {
    return generate_type(expression, expression->type());
}

llvm::Function *CodeGenerator::generate_function(ast::FunctionDefinition *definition) {
    std::map<types::ParameterType *, types::Type *> params;
    return generate_function(definition, params);
}

llvm::Function *CodeGenerator::generate_function(ast::FunctionDefinition *definition, std::map<types::ParameterType *, types::Type *> type_parameters) {
    auto &context = m_module->getContext();

    auto function_symbol = scope()->lookup(this, definition->name());
    auto function_type = static_cast<types::Function *>(function_symbol->type);

    auto method = static_cast<types::Method *>(definition->type());
    auto symbol = function_symbol->nameSpace->lookup_by_node(this, definition);

    std::string llvm_function_name = codegen::mangle_method(function_symbol->name, method);

    if (!type_parameters.empty()) {
        llvm_function_name += "_";
        for (auto entry : type_parameters) {
            auto t = entry.second;
            while (auto p = dynamic_cast<types::Parameter *>(t)) {
                t = get_type_parameter(p);
                if (t == nullptr) {
                    return nullptr;
                }
            }
            llvm_function_name += t->mangled_name();
        }
    }

    push_scope(symbol);

    for (auto entry : type_parameters) {
        push_type_parameter(entry.first, entry.second);
    }

    llvm::Type *llvmType = generate_type(definition);
    if (llvmType == nullptr) {
        return nullptr;
    }

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(llvmType);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_function_name, m_module);

    auto old_insert_point = m_ir_builder->saveIP();

    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(context, "entry", function);
    m_ir_builder->SetInsertPoint(basicBlock);

    for (ast::Name *param : definition->name()->parameters()) {
        auto s = scope()->lookup(this, definition, param->value());
        auto alloca = m_ir_builder->CreateAlloca(m_ir_builder->getInt1Ty(), 0, param->value());
        m_ir_builder->CreateStore(m_ir_builder->getInt1(false), alloca);
        s->value = alloca;
    }

    int i = 0;
    for (auto &arg : function->args()) {
        std::string arg_name = definition->parameters[i]->name->value();
        arg.setName(arg_name);

        llvm::Value *value = &arg;

        if (!definition->parameters[i]->inout) {
            auto alloca = m_ir_builder->CreateAlloca(arg.getType(), 0, arg_name);
            m_ir_builder->CreateStore(&arg, alloca);
            value = alloca;
        }

        auto arg_symbol = scope()->lookup(this, definition, arg_name);
        arg_symbol->value = value;

        i++;
    }

    definition->body->accept(this);

    auto value = pop_value();
    m_ir_builder->CreateRet(value);

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
        function->dump();
        report(InternalError(definition, stream.str()));
    }

    pop_scope();

    for (auto entry : type_parameters) {
        pop_type_parameter(entry.first);
    }

    m_ir_builder->restoreIP(old_insert_point);

    symbol->value = function;

    if (function_symbol->value == nullptr) {
      // check if global symbol is set
      auto llvm_function_type = generate_type(definition, function_type);
      if (llvm_function_type == nullptr) {
          return nullptr;
      }

      auto llvm_initialiser = take_initialiser(definition);

      auto variable = new llvm::GlobalVariable(*m_module, llvm_function_type, false,
                                               llvm::GlobalValue::InternalLinkage,
                                               llvm_initialiser, definition->name()->value());

      function_symbol->value = variable;
    }

    int index = 0;
    for (int i = 0; i < function_type->no_methods(); i++) {
        if (function_type->get_method(i) == method) {
            index = i;
        }
    }

    auto i32 = llvm::IntegerType::getInt32Ty(m_module->getContext());

    std::vector<llvm::Value *> indexes;
    indexes.push_back(llvm::ConstantInt::get(i32, 0));
    indexes.push_back(llvm::ConstantInt::get(i32, index));
    auto gep = m_ir_builder->CreateInBoundsGEP(function_symbol->value, indexes, "f");
    m_ir_builder->CreateStore(function, gep);

    return function;
}

llvm::BasicBlock *CodeGenerator::create_basic_block(std::string name) const {
    auto parent = m_ir_builder->GetInsertBlock()->getParent();
    return llvm::BasicBlock::Create(m_module->getContext(), name, parent);
}

void CodeGenerator::visit(ast::Block *block) {
    llvm::Value *last_value = nullptr;

    for (auto expression : block->expressions()) {
        expression->accept(this);
        last_value = pop_value();
    }

    push_value(last_value);
}

void CodeGenerator::visit(ast::Name *identifier) {
    auto symbol = scope()->lookup(this, identifier);
    if (symbol == nullptr) {
        push_value(nullptr);
        return;
    }

    if (!symbol->value) {
        report(InternalError(identifier, "should not be nullptr"));
        push_value(nullptr);
        return;
    }

    auto load = m_ir_builder->CreateLoad(symbol->value);
    push_value(load);
}

void CodeGenerator::visit(ast::VariableDeclaration *node) {
    auto symbol = scope()->lookup(this, node->name());

    auto llvm_type = generate_type(node);
    return_if_null(llvm_type);

    auto old_insert_point = m_ir_builder->saveIP();

    if (scope()->is_root()) {
        auto llvm_initialiser = take_initialiser(node);
        if (llvm_initialiser == nullptr) {
            return;
        }

        auto variable = new llvm::GlobalVariable(*m_module, llvm_type, false,
                                                 llvm::GlobalValue::CommonLinkage,
                                                 llvm_initialiser, node->name()->value());
        variable->setAlignment(4);
        variable->setVisibility(llvm::GlobalValue::DefaultVisibility);

        symbol->value = variable;

        auto insert_function = m_module->getFunction("_init_variables_");
        m_ir_builder->SetInsertPoint(&insert_function->getEntryBlock());
    } else {
        auto insert_function = m_ir_builder->GetInsertBlock()->getParent();
        m_ir_builder->SetInsertPoint(&insert_function->getEntryBlock().front());

        symbol->value = m_ir_builder->CreateAlloca(llvm_type, 0, node->name()->value());
    }

    m_ir_builder->restoreIP(old_insert_point);

    push_value(symbol->value);
}

void CodeGenerator::visit(ast::IntegerLiteral *expression) {
    auto type = generate_type(expression);

    uint64_t integer;
    std::stringstream ss;
    ss << expression->value();
    ss >> integer;

    push_value(llvm::ConstantInt::get(type, integer, true));
}

void CodeGenerator::visit(ast::FloatLiteral *expression) {
    auto type = generate_type(expression);

    double floatValue;
    std::stringstream ss;
    ss << expression->value();
    ss >> floatValue;

    push_value(llvm::ConstantFP::get(type, floatValue));
}

void CodeGenerator::visit(ast::ImaginaryLiteral *imaginary) {
    report(InternalError(imaginary, "N/A"));
    push_value(nullptr);
}

void CodeGenerator::visit(ast::StringLiteral *expression) {
    report(InternalError(expression, "N/A"));
    push_value(nullptr);
}

void CodeGenerator::visit(ast::SequenceLiteral *sequence) {
    std::vector<llvm::Value *> elements;

    for (auto element : sequence->elements) {
        element->accept(this);
        elements.push_back(pop_value());
    }

    llvm::StructType *type = static_cast<llvm::StructType *>(generate_type(sequence));

    llvm::Type *length_type = type->elements()[0];
    llvm::Type *element_type = type->elements()[0];

    // assign length
    llvm::Value *instance = m_ir_builder->CreateAlloca(type, nullptr, "array");

    llvm::Type *i32 = llvm::IntegerType::getInt32Ty(m_module->getContext());

    std::vector<llvm::Value *> length_index;
    length_index.push_back(llvm::ConstantInt::get(i32, 0));
    length_index.push_back(llvm::ConstantInt::get(i32, 0));

    std::vector<llvm::Value *> elements_index;
    elements_index.push_back(llvm::ConstantInt::get(i32, 0));
    elements_index.push_back(llvm::ConstantInt::get(i32, 1));

    llvm::Value *length = m_ir_builder->CreateInBoundsGEP(instance, length_index, "length");
    m_ir_builder->CreateStore(llvm::ConstantInt::get(length_type, elements.size()), length);

    auto elements_value = m_ir_builder->CreateInBoundsGEP(instance, elements_index, "elements");
    auto elements_instance = m_ir_builder->CreateAlloca(element_type, llvm::ConstantInt::get(i32, elements.size()));

    for (size_t i = 0; i < elements.size(); i++) {
        std::vector<llvm::Value *> index;
        index.push_back(llvm::ConstantInt::get(i32, i));
        auto place = m_ir_builder->CreateInBoundsGEP(elements_instance, index);
        m_ir_builder->CreateStore(elements[i], place);
    }

    m_ir_builder->CreateStore(elements_instance, elements_value);

    push_value(m_ir_builder->CreateLoad(instance));
}

void CodeGenerator::visit(ast::MappingLiteral *mapping) {
    report(InternalError(mapping, "N/A"));
    push_value(nullptr);
}

void CodeGenerator::visit(ast::RecordLiteral *expression) {
    //auto record = dynamic_cast<types::Record *>(expression->type);

    llvm::Type *llvm_type = generate_type(expression);
    if (llvm_type == nullptr) {
        push_value(nullptr);
        return;
    }

    auto instance = m_ir_builder->CreateAlloca(llvm_type);

    auto i32 = llvm::IntegerType::get(m_module->getContext(), 32);
    auto index0 = llvm::ConstantInt::get(i32, 0);
    for (size_t i = 0; i < expression->field_names.size(); i++) {
        auto index = llvm::ConstantInt::get(i32, i);

        expression->field_values[i]->accept(this);

        auto value = pop_value();

        std::vector<llvm::Value *> indexes;
        indexes.push_back(index0);
        indexes.push_back(index);

        auto ptr = m_ir_builder->CreateInBoundsGEP(instance, indexes);
        m_ir_builder->CreateStore(value, ptr);
    }

    push_value(m_ir_builder->CreateLoad(instance));
}

void CodeGenerator::visit(ast::TupleLiteral *expression) {
    auto llvm_type = generate_type(expression);

    auto instance = m_ir_builder->CreateAlloca(llvm_type);

    auto i32 = llvm::IntegerType::get(m_module->getContext(), 32);
    auto index0 = llvm::ConstantInt::get(i32, 0);

    auto elements = expression->elements();
    for (size_t i = 0; i < elements.size(); i++) {
        elements[i]->accept(this);
        auto value = pop_value();

        std::vector<llvm::Value *> indexes;
        indexes.push_back(index0);
        indexes.push_back(m_ir_builder->getInt32(i));

        auto ptr = m_ir_builder->CreateInBoundsGEP(instance, indexes);
        m_ir_builder->CreateStore(value, ptr);
    }

    push_value(m_ir_builder->CreateLoad(instance));
}

void CodeGenerator::visit(ast::Call *expression) {
    expression->operand->accept(this);

    auto function_type = dynamic_cast<types::Function *>(expression->operand->type());

    std::vector<types::Type *> argument_types;
    for (auto arg : expression->arguments) {
        argument_types.push_back(arg->type());
    }

    auto method = function_type->find_method(expression, argument_types);
    assert(method);

    std::cout << "found method: " << method->name() << std::endl;

    int index = function_type->index_of(method);

    auto ir_function = llvm::dyn_cast<llvm::LoadInst>(pop_value())->getPointerOperand();

    auto i32 = llvm::IntegerType::getInt32Ty(m_module->getContext());
    std::vector<llvm::Value *> indexes;
    indexes.push_back(llvm::ConstantInt::get(i32, 0));
    indexes.push_back(llvm::ConstantInt::get(i32, index));
    auto ir_method = m_ir_builder->CreateLoad(m_ir_builder->CreateInBoundsGEP(ir_function, indexes));

    if (ir_method == nullptr) {
        report(InternalError(expression, "No LLVM function was available!"));
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

    std::cout << "here" << std::endl;
    ir_method->dump();
    std::cout << "end here" << std::endl;

    auto value = m_ir_builder->CreateCall(ir_method, arguments);
    push_value(value);
}

void CodeGenerator::visit(ast::CCall *ccall) {
    std::vector<llvm::Type *> parameters;
    llvm::Type *returnType = generate_type(ccall);

    for (auto parameter : ccall->parameters) {
        parameters.push_back(generate_type(parameter));
    }

    std::string name = ccall->name->value();

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

    llvm::Value *value = m_ir_builder->CreateCall(function, arguments);
    push_value(value);
}

void CodeGenerator::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    auto value = pop_value();

    llvm::Type *destination_type = generate_type(cast);
    llvm::Value *new_value = m_ir_builder->CreateBitCast(value, destination_type);
    push_value(new_value);
}

void CodeGenerator::visit(ast::Assignment *expression) {
    expression->rhs->accept(this);
    auto rhs_value = pop_value();
    return_if_null(rhs_value);

    expression->lhs->accept(this);
    auto lhs_pointer = pop_value();

    /*} else if (rhs_union_type && !lhs_union_type) {
        bool ok;
        uint8_t index_we_want = rhs_union_type->type_index(expression->lhs->type, &ok);
        assert(ok);  // type checker should catch this

        std::vector<llvm::Value *> indexes;
        indexes.push_back(m_ir_builder->getInt32(0));
        indexes.push_back(m_ir_builder->getInt32(0));

        assert(rhs_variable_pointer);

        auto index_we_have_gep = m_ir_builder->CreateInBoundsGEP(rhs_variable_pointer, indexes, "union_index_ptr");
        auto index_we_have = m_ir_builder->CreateLoad(index_we_have_gep, "union_index");

        indexes.clear();
        indexes.push_back(m_ir_builder->getInt32(0));
        indexes.push_back(m_ir_builder->getInt32(1 + index_we_want));

        auto holder_gep = m_ir_builder->CreateInBoundsGEP(rhs_variable_pointer, indexes, "union_index_value_ptr");
        m_ir_builder->CreateStore(m_ir_builder->CreateLoad(holder_gep, "union_index"), variable_pointer);

        auto icmp = m_ir_builder->CreateICmpEQ(m_ir_builder->getInt8(index_we_want), index_we_have, "check_union_type");
        push_value(icmp);
    } else {*/

    lhs_pointer->dump();
    rhs_value->dump();

    m_ir_builder->CreateStore(rhs_value, lhs_pointer);
    push_value(rhs_value);
}

void CodeGenerator::visit(ast::Selector *expression) {
    expression->operand->accept(this);
    auto instance = pop_value();

    auto selectable = dynamic_cast<types::Selectable *>(expression->operand->type());
    assert(selectable);

    // either an enum, or a record
    auto record = dynamic_cast<types::Record *>(selectable);
    if (record) {
        uint64_t index = record->get_field_index(expression->name->value());

        std::vector<llvm::Value *> indexes;
        indexes.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(m_module->getContext(), 32), 0));
        indexes.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(m_module->getContext(), 32), index));

        llvm::Value *value = m_ir_builder->CreateInBoundsGEP(instance, indexes);
        push_value(value);
    } else {
        push_value(nullptr);
    }
}

void CodeGenerator::visit(ast::While *expression) {
    auto function = m_ir_builder->GetInsertBlock()->getParent();

    auto entry_bb = llvm::BasicBlock::Create(m_module->getContext(), "while_entry", function);
    m_ir_builder->CreateBr(entry_bb);

    m_ir_builder->SetInsertPoint(entry_bb);
    expression->condition()->accept(this);
    auto condition = m_ir_builder->CreateICmpEQ(pop_value(), m_ir_builder->getInt1(true), "while_cond");

    auto loop_bb = llvm::BasicBlock::Create(m_module->getContext(), "while_loop", function);
    auto else_bb = llvm::BasicBlock::Create(m_module->getContext(), "while_else", function);

    m_ir_builder->CreateCondBr(condition, loop_bb, else_bb);
    m_ir_builder->SetInsertPoint(loop_bb);

    expression->body()->accept(this);
    auto then_value = pop_value();
    push_value(then_value);
    m_ir_builder->CreateBr(entry_bb);

    m_ir_builder->SetInsertPoint(else_bb);
}

void CodeGenerator::visit(ast::If *expression) {
    expression->condition->accept(this);
    auto condition = pop_value();

    condition = m_ir_builder->CreateICmpEQ(condition, llvm::ConstantInt::get(llvm::Type::getInt1Ty(m_module->getContext()), 1), "ifcond");

    llvm::Function *function = m_ir_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *then_bb = llvm::BasicBlock::Create(m_module->getContext(), "then", function);
    llvm::BasicBlock *else_bb = llvm::BasicBlock::Create(m_module->getContext(), "else");
    llvm::BasicBlock *merge_bb = llvm::BasicBlock::Create(m_module->getContext(), "ifcont");

    m_ir_builder->CreateCondBr(condition, then_bb, else_bb);
    m_ir_builder->SetInsertPoint(then_bb);

    expression->true_case->accept(this);
    auto then_value = pop_value();

    m_ir_builder->CreateBr(merge_bb);

    then_bb = m_ir_builder->GetInsertBlock();

    function->getBasicBlockList().push_back(else_bb);
    m_ir_builder->SetInsertPoint(else_bb);

    llvm::Value *else_value = nullptr;
    if (expression->false_case) {
        expression->false_case->accept(this);
        else_value = pop_value();
    } else {
        else_value = m_ir_builder->getInt1(false);
    }

    m_ir_builder->CreateBr(merge_bb);

    else_bb = m_ir_builder->GetInsertBlock();

    function->getBasicBlockList().push_back(merge_bb);
    m_ir_builder->SetInsertPoint(merge_bb);

    llvm::Type *type = generate_type(expression);
    llvm::PHINode *phi = m_ir_builder->CreatePHI(type, 2, "iftmp");

    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    push_value(phi);
}

void CodeGenerator::visit(ast::Return *expression) {
    expression->expression->accept(this);
    auto value = pop_value();

    push_value(m_ir_builder->CreateRet(value));
}

void CodeGenerator::visit(ast::Spawn *expression) {
    report(InternalError(expression, "N/A"));
    push_value(nullptr);
}

void CodeGenerator::visit(ast::Switch *expression) {
    /*auto entry_block = create_basic_block("switch_entry");
    m_ir_builder->CreateBr(entry_block);

    m_ir_builder->SetInsertPoint(entry_block);
    expression->expression()->accept(this);
    auto condition = pop_value();

    int case_no = 0;
    for (auto case_ : expression->cases()) {
        std::stringstream ss;
        ss << "case_" << case_no;

        auto code_block = create_basic_block(ss.str());
        m_ir_builder->SetInsertPoint(code_block);

        case_no++;
    }

    auto exit_block = create_basic_block("switch_exit");
    m_ir_builder->SetInsertPoint(entry_block);*/

    report(InternalError(expression, "N/A"));
    push_value(nullptr);
}

void CodeGenerator::visit(ast::Parameter *parameter) {
    report(InternalError(parameter, "N/A"));
}

void CodeGenerator::visit(ast::VariableDefinition *definition) {
    definition->assignment->accept(this);
    definition->body()->accept(this);
}

void CodeGenerator::visit(ast::FunctionDefinition *definition) {
    if (!definition->name()->has_parameters()) {
        generate_function(definition);
    }

    push_value(nullptr);
}

void CodeGenerator::visit(ast::TypeDefinition *definition) {
    if (definition->alias) {
        auto new_symbol = scope()->lookup(this, definition->name());
        auto old_symbol = scope()->lookup(this, definition->alias);
        new_symbol->value = old_symbol->value;
    }

    push_value(nullptr);
}

void CodeGenerator::visit(ast::Import *statement) {
    report(InternalError(statement, "N/A"));
}

void CodeGenerator::visit(ast::SourceFile *module) {
    m_module = new llvm::Module(module->name, m_context);

    builtin_generate();

    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getVoidTy(m_module->getContext()), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "_init_variables_", m_module);
    llvm::BasicBlock *bb1 = llvm::BasicBlock::Create(m_module->getContext(), "entry", function);

    fType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_module->getContext()), false);
    llvm::Function *mainFunction = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "main", m_module);
    llvm::BasicBlock *main_bb = llvm::BasicBlock::Create(m_module->getContext(), "entry", mainFunction);

    m_ir_builder->SetInsertPoint(main_bb);
    m_ir_builder->CreateCall(m_module->getFunction("_init_builtins"));
    m_ir_builder->CreateCall(function);

    module->code->accept(this);
    assert(m_units.size() == 1);

    m_ir_builder->CreateRet(m_ir_builder->getInt32(0));

    m_ir_builder->SetInsertPoint(bb1);
    m_ir_builder->CreateRetVoid();

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyModule(*m_module, &stream)) {
        m_module->dump();
        report(InternalError(module, stream.str()));
    }
}

void CodeGenerator::builtin_generate() {
    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getVoidTy(m_module->getContext()), false);
    auto init_function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "_init_builtins", m_module);
    auto init_bb = llvm::BasicBlock::Create(m_module->getContext(), "entry", init_function);

    builtin_initialise_boolean_variable("nil", false);
    builtin_initialise_boolean_variable("true", true);
    builtin_initialise_boolean_variable("false", false);
    //initialise_boolean_variable(table, "Int32", false);
    //initialise_boolean_variable(table, "Int64", false);
    //initialise_boolean_variable(table, "UnsafePointer", false);

    // not
    llvm::Function *f = builtin_create_llvm_function("not", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateNot(m_args[0]));

    // multiplication
    f = builtin_create_llvm_function("*", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateMul(m_args[0], m_args[1]));

    // addition
    f = builtin_create_llvm_function("+", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateAdd(m_args[0], m_args[1]));

    f = builtin_create_llvm_function("+", 1);
    m_ir_builder->CreateRet(m_ir_builder->CreateAdd(m_args[0], m_args[1]));

    f = builtin_create_llvm_function("+", 2);
    m_ir_builder->CreateRet(m_ir_builder->CreateFAdd(m_args[0], m_args[1]));

    // subtraction
    f = builtin_create_llvm_function("-", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateSub(m_args[0], m_args[1]));

    // equality
    for (int i = 0; i < 3; i++) {
        f = builtin_create_llvm_function("==", i);
        m_ir_builder->CreateRet(m_ir_builder->CreateICmpEQ(m_args[0], m_args[1]));
    }

    // not equality
    f = builtin_create_llvm_function("!=", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateICmpNE(m_args[0], m_args[1]));

    // less than
    f = builtin_create_llvm_function("<", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateICmpSLT(m_args[0], m_args[1]));

    // greater than or equal to
    f = builtin_create_llvm_function(">=", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateICmpSGE(m_args[0], m_args[1]));

    // to integer
    f = builtin_create_llvm_function("to_integer", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateFPToSI(m_args[0], f->getReturnType()));

    // to float
    f = builtin_create_llvm_function("to_float", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateSIToFP(m_args[0], f->getReturnType()));

    m_ir_builder->SetInsertPoint(init_bb);
    m_ir_builder->CreateRetVoid();
}

llvm::Function *CodeGenerator::builtin_generate_function(std::string name, types::Method *method, std::string llvm_name) {
    method->accept(this);

    auto type = static_cast<llvm::FunctionType *>(take_type(nullptr));
    assert(type);

    auto old_insert_point = m_ir_builder->saveIP();

    auto function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, llvm_name, m_module);
    builtin_initialise_function(function, method->parameter_types().size());

    if (name == "sizeof") {
        builtin_generate_sizeof(method, function);
    } else if (name == "strideof") {
        builtin_generate_strideof(method, function);
    } else {
        std::vector<llvm::Value *> index_list;
        index_list.push_back(m_args[1]);
        auto gep = m_ir_builder->CreateInBoundsGEP(m_args[0], index_list);

        if (name == "setindex") {
            m_ir_builder->CreateStore(m_args[2], gep);
            m_ir_builder->CreateRet(m_ir_builder->getInt1(false));
        } else {
            m_ir_builder->CreateRet(m_ir_builder->CreateLoad(gep));
        }
    }

    m_ir_builder->restoreIP(old_insert_point);

    return function;
}

void CodeGenerator::builtin_generate_sizeof(types::Method *method, llvm::Function *function) {
    auto type = dynamic_cast<types::TypeType *>(method->parameter_types()[0]);
    type->create(nullptr, nullptr)->accept(this);
    auto llvm_type = take_type(nullptr);

    uint64_t size = m_data_layout->getTypeStoreSize(llvm_type);
    m_ir_builder->CreateRet(m_ir_builder->getInt64(size));
}

void CodeGenerator::builtin_generate_strideof(types::Method *method, llvm::Function *function) {
    auto type = dynamic_cast<types::TypeType *>(method->parameter_types()[0]);
    type->create(nullptr, nullptr)->accept(this);
    auto llvm_type = take_type(nullptr);

    uint64_t size = m_data_layout->getTypeAllocSize(llvm_type);
    m_ir_builder->CreateRet(m_ir_builder->getInt64(size));
}

llvm::Function *CodeGenerator::builtin_create_llvm_function(std::string name, int index) {
    auto function_symbol = scope()->lookup(nullptr, nullptr, name);

    auto functionType = static_cast<types::Function *>(function_symbol->type);
    auto methodType = functionType->get_method(index);

    std::string mangled_name = codegen::mangle_method(name, methodType);

    methodType->accept(this);

    llvm::FunctionType *type = static_cast<llvm::FunctionType *>(take_type(nullptr));
    assert(type);

    llvm::Function *f = llvm::Function::Create(type, llvm::Function::ExternalLinkage, mangled_name, m_module);
    f->addFnAttr(llvm::Attribute::AlwaysInline);

    if (function_symbol->value == nullptr) {
      // check if global symbol is set
      functionType->accept(this);

      auto llvm_function_type = take_type(nullptr);
      auto llvm_initialiser = take_initialiser(nullptr);

      auto variable = new llvm::GlobalVariable(*m_module, llvm_function_type, false,
                                               llvm::GlobalValue::InternalLinkage,
                                               llvm_initialiser, name);

      function_symbol->value = variable;
    }

    auto i32 = llvm::IntegerType::getInt32Ty(m_module->getContext());

    auto insert_function = m_module->getFunction("_init_builtins");
    m_ir_builder->SetInsertPoint(&insert_function->getEntryBlock());

    std::vector<llvm::Value *> indexes;
    indexes.push_back(llvm::ConstantInt::get(i32, 0));
    indexes.push_back(llvm::ConstantInt::get(i32, index));
    auto gep = m_ir_builder->CreateInBoundsGEP(function_symbol->value, indexes, "f");
    m_ir_builder->CreateStore(f, gep);

    builtin_initialise_function(f, methodType->parameter_types().size());

    return f;
}

void CodeGenerator::builtin_initialise_boolean_variable(std::string name, bool value) {
    auto symbol = scope()->lookup(nullptr, nullptr, name);
    symbol->value = new llvm::GlobalVariable(
        *m_module, m_ir_builder->getInt1Ty(), false,
        llvm::GlobalValue::InternalLinkage, m_ir_builder->getInt1(value), name
    );
}

void CodeGenerator::builtin_initialise_function(llvm::Function *function, int no_arguments) {
    m_args.clear();

    if (!function->arg_empty()) {
        m_args.push_back(&function->getArgumentList().front());
        for (size_t i = 1; i < function->arg_size(); i++) {
            m_args.push_back(function->getArgumentList().getNext(m_args[i - 1]));
        }
    }

    assert(m_args.size() == function->arg_size());
    assert(m_args.size() == no_arguments);

    auto &context = function->getContext();

    auto basic_block = llvm::BasicBlock::Create(context, "entry", function);
    m_ir_builder->SetInsertPoint(basic_block);
}
