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
#define return_and_push_null_if_null(thing) if (thing == nullptr) { push_llvm_value(nullptr); return; }
#define return_null_if_null(thing) if (thing == nullptr) return nullptr;

std::string codegen::mangle_method(std::string name, types::Method *type) {
    return "_A_" + name + "_" + type->mangled_name();
}

void ValueFollower::push_llvm_value(llvm::Value *value) {
    m_llvm_value_stack.push_back(value);
}

bool ValueFollower::has_llvm_value() const {
    return !m_llvm_value_stack.empty();
}

llvm::Value *ValueFollower::pop_llvm_value() {
    assert(has_llvm_value());
    auto value = m_llvm_value_stack.back();
    m_llvm_value_stack.pop_back();
    return value;
}

llvm::Value *ValueFollower::llvm_value() const {
    return m_llvm_value_stack.back();
}

void TypeFollower::push_llvm_type(llvm::Type *type) {
    m_llvm_type_stack.push_back(type);
}

llvm::Type *TypeFollower::pop_llvm_type() {
    assert(has_llvm_type());
    auto value = m_llvm_type_stack.back();
    m_llvm_type_stack.pop_back();
    return value;
}

bool TypeFollower::has_llvm_type() const {
    return !m_llvm_type_stack.empty();
}

llvm::Type *TypeFollower::llvm_type() const {
    return m_llvm_type_stack.back();
}

void InitialiserFollower::push_llvm_initialiser(llvm::Constant *initialiser) {
    m_llvm_initialiser_stack.push_back(initialiser);
}

llvm::Constant *InitialiserFollower::pop_llvm_initialiser() {
    assert(has_llvm_initialiser());
    auto value = m_llvm_initialiser_stack.back();
    m_llvm_initialiser_stack.pop_back();
    return value;
}

bool InitialiserFollower::has_llvm_initialiser() const {
    return !m_llvm_initialiser_stack.empty();
}

llvm::Constant *InitialiserFollower::llvm_initialiser() const {
    return m_llvm_initialiser_stack.back();
}

IrBuilder::IrBuilder(llvm::LLVMContext &context) : m_ir_builder(new llvm::IRBuilder<>(context)) {

}

llvm::BasicBlock *IrBuilder::create_basic_block(std::string name, llvm::Function *function) {
    if (function == nullptr) {
        function = m_ir_builder->GetInsertBlock()->getParent();
    }

    return llvm::BasicBlock::Create(function->getContext(), name, function);
}

llvm::BasicBlock *IrBuilder::create_entry_basic_block(llvm::Function *function) {
    return create_basic_block("entry", function);
}

std::vector<llvm::Value *> IrBuilder::build_gep_index(std::initializer_list<int> indexes) {
    std::vector<llvm::Value *> values;
    for (int index : indexes) {
        values.push_back(m_ir_builder->getInt32(index));
    }
    return values;
}

llvm::Value *IrBuilder::create_inbounds_gep(llvm::Value *value, std::initializer_list<int> indexes) {
    return m_ir_builder->CreateInBoundsGEP(value, build_gep_index(indexes));
}

llvm::Value *IrBuilder::create_store_method_to_function(llvm::Function *method, llvm::Value *function, int index) {
    auto method_holder = create_inbounds_gep(function, { 0, index });
    return m_ir_builder->CreateStore(method, method_holder);
}

CodeGenerator::CodeGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout) : IrBuilder(m_context) {
    push_scope(scope);

    m_md_builder = new llvm::MDBuilder(m_context);
    m_data_layout = data_layout;
}

CodeGenerator::~CodeGenerator() {
    delete m_ir_builder;
    delete m_md_builder;
}

llvm::Module *CodeGenerator::module() const {
    return m_module;
}

llvm::Type *CodeGenerator::take_type(ast::Expression *expression) {
    if (has_llvm_type()) {
        auto result = pop_llvm_type();

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
    if (has_llvm_initialiser()) {
        auto result = pop_llvm_initialiser();

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

llvm::Type *CodeGenerator::generate_type(ast::Expression *expression, types::Type *type) {
    type->accept(this);
    return take_type(expression);
}

llvm::Type *CodeGenerator::generate_type(ast::Expression *expression) {
    return generate_type(expression, expression->type());
}

void CodeGenerator::push_llvm_type_and_initialiser(llvm::Type *type, llvm::Constant *initialiser) {
    push_llvm_type(type);
    push_llvm_initialiser(initialiser);
}

void CodeGenerator::push_null_llvm_type_and_initialiser() {
    push_llvm_type_and_initialiser(nullptr, nullptr);
}

void CodeGenerator::builtin_generate() {
    builtin_initialise_boolean_variable("nil", false);
    builtin_initialise_boolean_variable("true", true);
    builtin_initialise_boolean_variable("false", false);

    // not
    auto f = builtin_create_llvm_function("not", 0);
    m_ir_builder->CreateRet(m_ir_builder->CreateNot(m_args[0]));

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
}

llvm::Function *CodeGenerator::builtin_generate_function(std::string name, types::Method *method, std::string llvm_name) {
    method->accept(this);

    auto llvm_function_type = static_cast<llvm::FunctionType *>(take_type(nullptr));
    return_null_if_null(llvm_function_type);

    auto old_insert_point = m_ir_builder->saveIP();

    auto function = llvm::Function::Create(llvm_function_type, llvm::Function::ExternalLinkage, llvm_name, m_module);
    builtin_initialise_function(function, method->parameter_types().size());

    if (name == "sizeof") {
        builtin_generate_sizeof(method, function);
    } else if (name == "strideof") {
        builtin_generate_strideof(method, function);
    } else {
        auto gep = m_ir_builder->CreateInBoundsGEP(m_args[0], { m_args[0] });

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
    auto llvm_type = generate_type(nullptr, type->create(nullptr, nullptr));
    return_if_null(llvm_type);

    uint64_t size = m_data_layout->getTypeStoreSize(llvm_type);
    m_ir_builder->CreateRet(m_ir_builder->getInt64(size));
}

void CodeGenerator::builtin_generate_strideof(types::Method *method, llvm::Function *function) {
    auto type = dynamic_cast<types::TypeType *>(method->parameter_types()[0]);
    auto llvm_type = generate_type(nullptr, type->create(nullptr, nullptr));
    return_if_null(llvm_type);

    uint64_t size = m_data_layout->getTypeAllocSize(llvm_type);
    m_ir_builder->CreateRet(m_ir_builder->getInt64(size));
}

llvm::Function *CodeGenerator::builtin_create_llvm_function(std::string name, int index) {
    auto function_symbol = scope()->lookup(nullptr, nullptr, name);

    auto function_type = static_cast<types::Function *>(function_symbol->type);
    auto method_type = function_type->get_method(index);

    std::string mangled_name = codegen::mangle_method(name, method_type);

    auto llvm_type = generate_type(nullptr, method_type);
    auto llvm_type_for_method = static_cast<llvm::FunctionType *>(llvm_type);
    return_null_if_null(llvm_type_for_method);

    auto function = llvm::Function::Create(llvm_type_for_method, llvm::Function::ExternalLinkage, mangled_name, m_module);
    function->addFnAttr(llvm::Attribute::AlwaysInline);

    if (function_symbol->value == nullptr) {
      auto llvm_type_for_function = generate_type(nullptr, function_type);
      return_null_if_null(llvm_type_for_function);
      auto llvm_initialiser_for_function = take_initialiser(nullptr);
      return_null_if_null(llvm_initialiser_for_function);

      auto variable = new llvm::GlobalVariable(
          *m_module, llvm_type_for_function, false,
          llvm::GlobalValue::InternalLinkage, llvm_initialiser_for_function,
          name
      );

      function_symbol->value = variable;
    }

    m_ir_builder->SetInsertPoint(&m_init_builtins_function->getEntryBlock());

    create_store_method_to_function(function, function_symbol->value, index);

    builtin_initialise_function(function, method_type->parameter_types().size());

    return function;
}

void CodeGenerator::builtin_initialise_boolean_variable(std::string name, bool value) {
    auto symbol = scope()->lookup(nullptr, nullptr, name);
    return_if_null(symbol);
    symbol->value = new llvm::GlobalVariable(
        *m_module, m_ir_builder->getInt1Ty(), false,
        llvm::GlobalValue::InternalLinkage, m_ir_builder->getInt1(value), name
    );
}

void CodeGenerator::builtin_initialise_function(llvm::Function *function, int no_arguments) {
    m_args.clear();
    for (auto &arg : function->getArgumentList()) {
        m_args.push_back(&arg);
    }

    if (static_cast<int>(m_args.size()) != no_arguments) {
        report(InternalError(nullptr, "Builtin given number of arguments doesn't match actual."));
        return;
    }

    auto basic_block = create_entry_basic_block(function);
    m_ir_builder->SetInsertPoint(basic_block);
}

llvm::Value *CodeGenerator::generate_llvm_value(ast::Node *node) {
    node->accept(this);
    auto value = pop_llvm_value();

    if (value == nullptr) {
        report(InternalError(node, "No LLVM value generated!"));
    }

    return value;
}

void CodeGenerator::visit_constructor(types::TypeType *type) {
    push_llvm_type_and_initialiser(
        m_ir_builder->getInt1Ty(),
        m_ir_builder->getInt1(0)
    );
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
    type->constructor()->accept(this);

    std::vector<llvm::Type *> struct_fields;
    struct_fields.push_back(pop_llvm_type());

    std::vector<llvm::Constant *> struct_initialisers;
    struct_initialisers.push_back(pop_llvm_initialiser());

    auto llvm_type = llvm::StructType::get(m_context, struct_fields);
    auto llvm_initialiser = llvm::ConstantStruct::get(llvm_type, struct_initialisers);

    push_llvm_type_and_initialiser(llvm_type, llvm_initialiser);
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
    push_null_llvm_type_and_initialiser();
}

void CodeGenerator::visit(types::Void *type) {
    push_llvm_type_and_initialiser(
        m_ir_builder->getInt1Ty(),
        m_ir_builder->getInt1(0)
    );
}

void CodeGenerator::visit(types::Boolean *type) {
    push_llvm_type_and_initialiser(
        m_ir_builder->getInt1Ty(),
        m_ir_builder->getInt1(0)
    );
}

void CodeGenerator::visit(types::Integer *type) {
    const unsigned int size = type->size();
    push_llvm_type_and_initialiser(
        m_ir_builder->getIntNTy(size),
        m_ir_builder->getIntN(size, 0)
    );
}

void CodeGenerator::visit(types::UnsignedInteger *type) {
    const unsigned int size = type->size();
    push_llvm_type_and_initialiser(
        m_ir_builder->getIntNTy(size),
        m_ir_builder->getIntN(size, 0)
    );
}

void CodeGenerator::visit(types::Float *type) {
    const unsigned int size = type->size();
    llvm::Type *llvm_type = nullptr;

    switch (size) {
        case 64:
            llvm_type = m_ir_builder->getDoubleTy();
            break;
        case 32:
            llvm_type = m_ir_builder->getFloatTy();
            break;
        case 16:
            llvm_type = m_ir_builder->getHalfTy();
            break;
    }

    push_llvm_type_and_initialiser(
        llvm_type, llvm::ConstantFP::get(llvm_type, 0)
    );
}

void CodeGenerator::visit(types::UnsafePointer *type) {
    auto llvm_element_type = generate_type(nullptr, type->element_type());
    if (llvm_element_type == nullptr) {
        push_null_llvm_type_and_initialiser();
        return;
    }

    auto llvm_pointer_type = llvm::PointerType::getUnqual(llvm_element_type);
    push_llvm_type(llvm_pointer_type);
    push_llvm_initialiser(llvm::ConstantPointerNull::get(llvm_pointer_type));
}

void CodeGenerator::visit(types::Record *type) {
    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (auto field_type : type->field_types()) {
        auto llvm_field_type = generate_type(nullptr, field_type);
        auto llvm_field_initialiser = take_initialiser(nullptr);

        if (llvm_field_type == nullptr || llvm_field_initialiser == nullptr) {
            push_null_llvm_type_and_initialiser();
            return;
        }

        llvm_types.push_back(llvm_field_type);
        llvm_initialisers.push_back(llvm_field_initialiser);
    }

    auto struct_type = llvm::StructType::get(m_context, llvm_types);
    auto struct_initialiser = llvm::ConstantStruct::get(struct_type, llvm_initialisers);

    push_llvm_type(struct_type);
    push_llvm_initialiser(struct_initialiser);
}

void CodeGenerator::visit(types::Tuple *type) {
    visit(static_cast<types::Record *>(type));
}

void CodeGenerator::visit(types::Method *type) {
    auto llvm_return_type = generate_type(nullptr, type->return_type());
    if (llvm_return_type == nullptr) {
        push_null_llvm_type_and_initialiser();
        return;
    }

    std::vector<llvm::Type *> llvm_parameter_types;
    for (auto parameter_type : type->parameter_types()) {
        auto llvm_parameter_type = generate_type(nullptr, parameter_type);
        if (llvm_parameter_type == nullptr) {
            push_null_llvm_type_and_initialiser();
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
    push_llvm_type(llvm_type);
    push_llvm_initialiser(nullptr);
}

void CodeGenerator::visit(types::Function *type) {
    std::vector<llvm::Type *> function_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (int i = 0; i < type->no_methods(); i++) {
        auto method = type->get_method(i);
        auto llvm_type_for_method = generate_type(nullptr, method);
        auto pointer_type = llvm::PointerType::getUnqual(llvm_type_for_method);
        function_types.push_back(pointer_type);
        llvm_initialisers.push_back(llvm::ConstantPointerNull::get(pointer_type));
    }

    auto llvm_type = llvm::StructType::get(m_context, function_types);
    auto struct_initialiser = llvm::ConstantStruct::get(llvm_type, llvm_initialisers);

    push_llvm_type_and_initialiser(llvm_type, struct_initialiser);
}

void CodeGenerator::visit(types::Module *type) {
    push_null_llvm_type_and_initialiser();
}

void CodeGenerator::visit(ast::Block *block) {
    llvm::Value *last_value = nullptr;

    for (auto expression : block->expressions()) {
        expression->accept(this);
        last_value = pop_llvm_value();
        // FIXME return_and_push_null_if_null(last_value);
    }

    push_llvm_value(last_value);
}

void CodeGenerator::visit(ast::Name *name) {
    auto symbol = scope()->lookup(this, name);
    return_and_push_null_if_null(symbol);

    if (symbol->value == nullptr) {
        report(InternalError(name, "The LLVM value of this symbol is null."));
        push_llvm_value(nullptr);
        return;
    }

    push_llvm_value(m_ir_builder->CreateLoad(symbol->value));
}

void CodeGenerator::visit(ast::VariableDeclaration *node) {
    auto symbol = scope()->lookup(this, node->name());

    auto llvm_type = generate_type(node);
    return_and_push_null_if_null(llvm_type);

    auto old_insert_point = m_ir_builder->saveIP();

    if (scope()->is_root()) {
        auto llvm_initialiser = take_initialiser(node);
        return_and_push_null_if_null(llvm_initialiser);

        auto variable = new llvm::GlobalVariable(*m_module, llvm_type, false,
                                                 llvm::GlobalValue::CommonLinkage,
                                                 llvm_initialiser, node->name()->value());
        variable->setAlignment(4);
        variable->setVisibility(llvm::GlobalValue::DefaultVisibility);

        symbol->value = variable;

        m_ir_builder->SetInsertPoint(&m_init_variables_function->getEntryBlock());
    } else {
        auto insert_function = m_ir_builder->GetInsertBlock()->getParent();
        m_ir_builder->SetInsertPoint(&insert_function->getEntryBlock().front());

        symbol->value = m_ir_builder->CreateAlloca(llvm_type, 0, node->name()->value());
    }

    m_ir_builder->restoreIP(old_insert_point);

    push_llvm_value(symbol->value);
}

void CodeGenerator::visit(ast::Int *expression) {
    auto type = generate_type(expression);

    uint64_t integer;
    std::stringstream ss;
    ss << expression->value();
    ss >> integer;

    push_llvm_value(llvm::ConstantInt::get(type, integer, true));
}

void CodeGenerator::visit(ast::Float *expression) {
    auto type = generate_type(expression);

    double floatValue;
    std::stringstream ss;
    ss << expression->value();
    ss >> floatValue;

    push_llvm_value(llvm::ConstantFP::get(type, floatValue));
}

void CodeGenerator::visit(ast::Complex *imaginary) {
    report(InternalError(imaginary, "Not yet implemented."));
    push_llvm_value(nullptr);
}

void CodeGenerator::visit(ast::String *expression) {
    report(InternalError(expression, "Not yet implemented."));
    push_llvm_value(nullptr);
}

void CodeGenerator::visit(ast::List *sequence) {
    std::vector<llvm::Value *> elements;
    for (auto element : sequence->elements()) {
        element->accept(this);
        auto value = pop_llvm_value();
        return_and_push_null_if_null(value);
        elements.push_back(value);
    }

    auto type = static_cast<llvm::StructType *>(generate_type(sequence));
    return_and_push_null_if_null(type);

    auto length_type = type->elements()[0];
    auto element_type = type->elements()[1];

    // assign length
    auto instance = m_ir_builder->CreateAlloca(type, nullptr, "array");

    auto length_index = build_gep_index({ 0, 0 });
    auto elements_index = build_gep_index({ 0, 1 });

    auto length = m_ir_builder->CreateInBoundsGEP(instance, length_index, "length");
    m_ir_builder->CreateStore(llvm::ConstantInt::get(length_type, elements.size()), length);

    auto elements_value = m_ir_builder->CreateInBoundsGEP(instance, elements_index, "elements");
    auto elements_instance = m_ir_builder->CreateAlloca(element_type, m_ir_builder->getInt32(elements.size()));

    for (int i = 0; i < static_cast<int>(elements.size()); i++) {
        auto index = build_gep_index({ i });
        auto place = m_ir_builder->CreateInBoundsGEP(elements_instance, index);
        m_ir_builder->CreateStore(elements[i], place);
    }

    m_ir_builder->CreateStore(elements_instance, elements_value);

    push_llvm_value(m_ir_builder->CreateLoad(instance));
}

void CodeGenerator::visit(ast::Dictionary *mapping) {
    report(InternalError(mapping, "N/A"));
    push_llvm_value(nullptr);
}

void CodeGenerator::visit(ast::Tuple *expression) {
    auto llvm_type = generate_type(expression);

    auto instance = m_ir_builder->CreateAlloca(llvm_type);

    auto elements = expression->elements();
    for (int i = 0; i < static_cast<int>(elements.size()); i++) {
        elements[i]->accept(this);
        auto value = pop_llvm_value();
        return_if_null(value);

        auto ptr = m_ir_builder->CreateInBoundsGEP(instance, build_gep_index({ 0, i }));
        m_ir_builder->CreateStore(value, ptr);
    }

    push_llvm_value(m_ir_builder->CreateLoad(instance));
}

void CodeGenerator::visit(ast::Call *node) {
    node->operand->accept(this);

    auto function_type = dynamic_cast<types::Function *>(node->operand->type());

    auto method_index = node->get_method_index();
    auto method = function_type->get_method(method_index);

    if (method->is_generic()) {
        method_index += node->get_method_generic_specialisation_index();
    }

    auto ir_function = llvm::dyn_cast<llvm::LoadInst>(pop_llvm_value())->getPointerOperand();

    auto ir_method = m_ir_builder->CreateLoad(m_ir_builder->CreateInBoundsGEP(ir_function, build_gep_index({ 0, method_index })));

    if (ir_method == nullptr) {
        report(InternalError(node, "No LLVM function was available!"));
        push_llvm_value(nullptr);
        return;
    }

    std::vector<llvm::Value *> arguments;
    int i = 0;
    bool valid;
    for (auto argument : method->ordered_arguments(node, &valid)) {
        argument->accept(this);

        auto value = pop_llvm_value();

        if (method->is_parameter_inout(method->parameter_types()[i])) {
            auto load = llvm::dyn_cast<llvm::LoadInst>(value);
            assert(load);

            value = load->getPointerOperand();
        }

        arguments.push_back(value);
        i++;
    }

    if (!valid) {
        report(InternalError(node, "Could not order arguments!"));
        push_llvm_value(nullptr);
        return;
    }

    debug("here");
    ir_method->dump();
    debug("end here");

    auto call = m_ir_builder->CreateCall(ir_method, arguments);
    push_llvm_value(call);
}

void CodeGenerator::visit(ast::CCall *ccall) {
    std::vector<llvm::Type *> parameters;
    auto return_type = generate_type(ccall);
    for (auto parameter : ccall->parameters) {
        parameters.push_back(generate_type(parameter));
    }

    std::string name = ccall->name->value();

    auto function_type = llvm::FunctionType::get(
        return_type, parameters, false
    );

    auto function = m_module->getFunction(name);
    if (!function) {
        function = llvm::Function::Create(
            function_type, llvm::Function::ExternalLinkage, name, m_module
        );
    }

    // TODO check duplication signature matches

    std::vector<llvm::Value *> arguments;
    for (auto argument : ccall->arguments) {
        argument->accept(this);
        auto arg_value = pop_llvm_value();
        return_and_push_null_if_null(arg_value);
        arguments.push_back(arg_value);
    }

    auto call = m_ir_builder->CreateCall(function, arguments);
    push_llvm_value(call);
}

void CodeGenerator::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    auto value = pop_llvm_value();

    auto destination_type = generate_type(cast);
    auto new_value = m_ir_builder->CreateBitCast(value, destination_type);
    push_llvm_value(new_value);
}

void CodeGenerator::visit(ast::Assignment *expression) {
    expression->rhs->accept(this);
    auto rhs_value = pop_llvm_value();
    return_if_null(rhs_value);

    expression->lhs->accept(this);
    auto lhs_pointer = pop_llvm_value();

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
        push_llvm_value(icmp);
    } else {*/

    lhs_pointer->dump();
    rhs_value->dump();

    m_ir_builder->CreateStore(rhs_value, lhs_pointer);
    push_llvm_value(rhs_value);
}

void CodeGenerator::visit(ast::Selector *expression) {
    auto module_type = dynamic_cast<types::Module *>(expression->operand->type());
    auto record_type_type = dynamic_cast<types::RecordType *>(expression->operand->type());
    auto record_type = dynamic_cast<types::Record *>(expression->operand->type());

    if (module_type) {
        auto module_name = static_cast<ast::Name *>(expression->operand);

        auto symbol = scope()->lookup(this, module_name);
        return_and_push_null_if_null(symbol);

        push_scope(symbol);
        expression->name->accept(this);
        pop_scope();
    } else if (record_type_type) {
        expression->operand->accept(this);
        auto instance = pop_llvm_value();

        if (expression->name->value() == "new") {
            push_llvm_value(instance);
        } else {
            report(InternalError(expression, "unsupported selector"));
            push_llvm_value(nullptr);
        }
    } else if (record_type) {
        expression->operand->accept(this);
        auto instance = pop_llvm_value();

        auto actual_thing = llvm::dyn_cast<llvm::LoadInst>(instance)->getPointerOperand();

        auto record = dynamic_cast<types::Record *>(expression->operand->type());

        int index = record->get_field_index(expression->name->value());
        auto value = m_ir_builder->CreateLoad(m_ir_builder->CreateInBoundsGEP(actual_thing, build_gep_index({ 0, index })));
        push_llvm_value(value);
    } else {
        report(InternalError(expression, "unsupported selector"));
        push_llvm_value(nullptr);
    }
}

void CodeGenerator::visit(ast::While *expression) {
    auto entry_bb = create_basic_block("while_entry");
    auto loop_bb = create_basic_block("while_loop");
    auto join_bb = create_basic_block("while_join");

    m_ir_builder->CreateBr(entry_bb);

    m_ir_builder->SetInsertPoint(entry_bb);
    expression->condition()->accept(this);
    auto condition = m_ir_builder->CreateICmpEQ(pop_llvm_value(), m_ir_builder->getInt1(true), "while_cond");
    m_ir_builder->CreateCondBr(condition, loop_bb, join_bb);

    m_ir_builder->SetInsertPoint(loop_bb);

    expression->body()->accept(this);
    auto then_value = pop_llvm_value();
    push_llvm_value(then_value);
    m_ir_builder->CreateBr(entry_bb);

    m_ir_builder->SetInsertPoint(join_bb);
}

void CodeGenerator::visit(ast::If *expression) {
    auto condition = generate_llvm_value(expression->condition);
    return_and_push_null_if_null(condition);

    condition = m_ir_builder->CreateICmpEQ(condition, m_ir_builder->getTrue(), "if_cond");

    auto then_bb = create_basic_block("if_then");
    auto else_bb = create_basic_block("if_else");
    auto join_bb = create_basic_block("if_join");

    m_ir_builder->CreateCondBr(condition, then_bb, else_bb);
    m_ir_builder->SetInsertPoint(then_bb);

    auto then_value = generate_llvm_value(expression->true_case);
    return_and_push_null_if_null(then_value);
    m_ir_builder->CreateBr(join_bb);

    then_bb = m_ir_builder->GetInsertBlock();

    m_ir_builder->SetInsertPoint(else_bb);

    llvm::Value *else_value = nullptr;
    if (expression->false_case) {
        else_value = generate_llvm_value(expression->false_case);
    } else {
        else_value = m_ir_builder->getInt1(false);
    }
    return_and_push_null_if_null(else_value);
    m_ir_builder->CreateBr(join_bb);

    else_bb = m_ir_builder->GetInsertBlock();

    m_ir_builder->SetInsertPoint(join_bb);

    auto type = generate_type(expression);
    return_and_push_null_if_null(type);

    auto phi = m_ir_builder->CreatePHI(type, 2, "iftmp");
    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    push_llvm_value(phi);
}

void CodeGenerator::visit(ast::Return *expression) {
    auto value = generate_llvm_value(expression->expression);
    return_and_push_null_if_null(value);
    push_llvm_value(m_ir_builder->CreateRet(value));
}

void CodeGenerator::visit(ast::Spawn *expression) {
    report(InternalError(expression, "N/A"));
    push_llvm_value(nullptr);
}

void CodeGenerator::visit(ast::Switch *expression) {
    /*auto entry_block = create_basic_block("switch_entry");
    m_ir_builder->CreateBr(entry_block);

    m_ir_builder->SetInsertPoint(entry_block);
    expression->expression()->accept(this);
    auto condition = pop_llvm_value();

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
    push_llvm_value(nullptr);
}

void CodeGenerator::visit(ast::Parameter *parameter) {
    report(InternalError(parameter, "N/A"));
}

void CodeGenerator::visit(ast::Let *definition) {
    definition->assignment->accept(this);

    if (definition->has_body()) {
        definition->body()->accept(this);
    }
}

void CodeGenerator::visit(ast::Def *definition) {
    /*if (definition->name()->has_parameters()) {
        report(InternalError(definition, "Function should not have definitions."));
        push_llvm_value(nullptr);
        return;
    }*/

    auto function_symbol = scope()->lookup(this, definition->name());
    auto function_type = static_cast<types::Function *>(function_symbol->type);

    auto method = static_cast<types::Method *>(definition->type());
    auto symbol = function_symbol->nameSpace->lookup_by_node(this, definition);

    auto llvm_function_name = codegen::mangle_method(function_symbol->name, method);

    push_scope(symbol);

    if (method->is_generic()) {
        std::cout << method->generic_specialisations().size() << std::endl;
    }

    auto llvm_type = generate_type(definition);
    return_and_push_null_if_null(llvm_type);

    auto llvm_function_type = static_cast<llvm::FunctionType *>(llvm_type);
    auto function = llvm::Function::Create(llvm_function_type, llvm::Function::ExternalLinkage, llvm_function_name, m_module);

    auto old_insert_point = m_ir_builder->saveIP();

    auto entry_bb = create_entry_basic_block(function);
    m_ir_builder->SetInsertPoint(entry_bb);

    for (auto param : definition->name()->parameters()) {
        auto s = scope()->lookup(this, definition, param->value());
        auto alloca = m_ir_builder->CreateAlloca(m_ir_builder->getInt1Ty(), 0, param->value());
        m_ir_builder->CreateStore(m_ir_builder->getInt1(false), alloca);
        s->value = alloca;
    }

    int i = 0;
    for (auto &arg : function->args()) {
        std::string arg_name = definition->get_parameter(i)->name()->value();
        arg.setName(arg_name);

        llvm::Value *value = &arg;

        if (!definition->get_parameter(i)->inout()) {
            auto alloca = m_ir_builder->CreateAlloca(arg.getType(), 0, arg_name);
            m_ir_builder->CreateStore(&arg, alloca);
            value = alloca;
        }

        auto arg_symbol = scope()->lookup(this, definition, arg_name);
        arg_symbol->value = value;

        i++;
    }

    if (definition->builtin()) {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, definition, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, definition, "b")->value);
        push_llvm_value(m_ir_builder->CreateMul(a_value, b_value, "multiplication"));
    } else {
        definition->body()->accept(this);
    }

    auto value = pop_llvm_value();
    return_and_push_null_if_null(value);

    m_ir_builder->CreateRet(value);

    pop_scope();

    m_ir_builder->restoreIP(old_insert_point);

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
        function->dump();
        report(InternalError(definition, stream.str()));
        push_llvm_value(nullptr);
        return;
    }

    symbol->value = function;

    if (function_symbol->value == nullptr) {
      // check if global symbol is set
      auto llvm_function_type = generate_type(definition, function_type);
      return_and_push_null_if_null(llvm_function_type);

      auto llvm_initialiser = take_initialiser(definition);
      return_and_push_null_if_null(llvm_initialiser);

      auto variable = new llvm::GlobalVariable(*m_module, llvm_function_type, false,
                                               llvm::GlobalValue::InternalLinkage,
                                               llvm_initialiser, definition->name()->value());

      function_symbol->value = variable;
    }

    int index = function_type->index_of(method);
    create_store_method_to_function(function, function_symbol->value, index);

    push_llvm_value(function);
}

void CodeGenerator::visit(ast::Type *definition) {
    if (definition->alias) {
        auto new_symbol = scope()->lookup(this, definition->name());
        auto old_symbol = scope()->lookup(this, definition->alias);
        new_symbol->value = old_symbol->value;
        push_llvm_value(new_symbol->value);
    } else {
        auto node_type = dynamic_cast<types::RecordType *>(definition->type());

        auto symbol = scope()->lookup(this, definition->name());
        return_and_push_null_if_null(symbol);

        auto llvm_type = generate_type(definition, node_type->constructor());
        return_and_push_null_if_null(llvm_type);

        auto llvm_initialiser = take_initialiser(definition);
        return_and_push_null_if_null(llvm_initialiser);

        // variable to hold the type
        auto variable = new llvm::GlobalVariable(
            *m_module, llvm_type, false, llvm::GlobalValue::InternalLinkage,
            llvm_initialiser, definition->name()->value()
        );

        symbol->value = variable;

        // create constructor function
        auto function_type = node_type->constructor();
        auto method_type = function_type->get_method(0);

        std::string mangled_name = codegen::mangle_method(definition->name()->value(), method_type);

        auto llvm_type_for_method = static_cast<llvm::FunctionType *>(generate_type(nullptr, method_type));
        return_and_push_null_if_null(llvm_type_for_method);

        auto method = llvm::Function::Create(
            llvm_type_for_method, llvm::Function::ExternalLinkage,
            mangled_name, m_module
        );

        method->addFnAttr(llvm::Attribute::AlwaysInline);

        auto function = variable;
        create_store_method_to_function(method, function, 0);

        auto old_insert_point = m_ir_builder->saveIP();

        auto entry_bb = create_entry_basic_block(method);
        m_ir_builder->SetInsertPoint(entry_bb);

        auto instance = m_ir_builder->CreateAlloca(llvm_type_for_method->getReturnType());

        int i = 0;
        for (auto &arg : method->getArgumentList()) {
            auto ptr = create_inbounds_gep(instance, { 0, i });
            m_ir_builder->CreateStore(&arg, ptr);
            i++;
        }

        m_ir_builder->CreateRet(m_ir_builder->CreateLoad(instance));

        m_ir_builder->restoreIP(old_insert_point);

        push_llvm_value(variable);
    }
}

void CodeGenerator::visit(ast::Module *module) {
    auto symbol = scope()->lookup(this, module->name());
    return_and_push_null_if_null(symbol);

    push_scope(symbol);
    module->body()->accept(this);
    pop_scope();
}

void CodeGenerator::visit(ast::Import *statement) {
    report(InternalError(statement, "N/A"));
}

void CodeGenerator::visit(ast::SourceFile *module) {
    m_module = new llvm::Module(module->name, m_context);

    auto void_function_type = llvm::FunctionType::get(m_ir_builder->getVoidTy(), false);

    m_init_builtins_function = llvm::Function::Create(void_function_type, llvm::Function::ExternalLinkage, "_init_builtins_", m_module);
    auto init_builtins_bb = create_entry_basic_block(m_init_builtins_function);
    m_ir_builder->SetInsertPoint(init_builtins_bb);
    builtin_generate();
    m_ir_builder->SetInsertPoint(init_builtins_bb);
    m_ir_builder->CreateRetVoid();

    m_init_variables_function = llvm::Function::Create(void_function_type, llvm::Function::ExternalLinkage, "_init_variables_", m_module);
    auto init_variables_bb = create_entry_basic_block(m_init_variables_function);

    auto user_code_function = llvm::Function::Create(void_function_type, llvm::Function::ExternalLinkage, "_user_code_", m_module);
    auto user_code_bb = create_entry_basic_block(user_code_function);

    auto int32_function_type = llvm::FunctionType::get(m_ir_builder->getInt32Ty(), false);
    auto main_function = llvm::Function::Create(int32_function_type, llvm::Function::ExternalLinkage, "main", m_module);
    auto main_bb = create_entry_basic_block(main_function);

    m_ir_builder->SetInsertPoint(user_code_bb);
    module->code->accept(this);
    m_ir_builder->CreateRetVoid();

    m_ir_builder->SetInsertPoint(main_bb);
    m_ir_builder->CreateCall(m_init_builtins_function);
    m_ir_builder->CreateCall(m_init_variables_function);
    m_ir_builder->CreateCall(user_code_function);
    m_ir_builder->CreateRet(m_ir_builder->getInt32(0));

    m_ir_builder->SetInsertPoint(init_variables_bb);
    m_ir_builder->CreateRetVoid();

    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyModule(*m_module, &stream)) {
        m_module->dump();
        report(InternalError(module, stream.str()));
    }
}
