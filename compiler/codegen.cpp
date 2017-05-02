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

std::string codegen::mangle(std::string name) {
    return "_A_" + name;
}

std::string codegen::mangle_method(std::string name, types::Method *type) {
    return mangle(name + "_" + type->mangled_name());
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

void IrBuilder::push_insert_point() {
    m_insert_points.push_back(m_ir_builder->saveIP());
}

void IrBuilder::pop_insert_point() {
    m_ir_builder->restoreIP(insert_point());
    m_insert_points.pop_back();
}

llvm::IRBuilderBase::InsertPoint IrBuilder::insert_point() const {
    return m_insert_points.back();
}

llvm::BasicBlock *IrBuilder::create_basic_block(std::string name, llvm::Function *function, bool set_insert_point) {
    if (function == nullptr) {
        function = m_ir_builder->GetInsertBlock()->getParent();
    }

    auto bb = llvm::BasicBlock::Create(function->getContext(), name, function);

    if (set_insert_point) {
        m_ir_builder->SetInsertPoint(bb);
    }

    return bb;
}

llvm::BasicBlock *IrBuilder::create_entry_basic_block(llvm::Function *function, bool set_insert_point) {
    return create_basic_block("entry", function, set_insert_point);
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

llvm::Value *IrBuilder::create_store_method_to_function(llvm::Function *method, llvm::Value *function, int method_index, int specialisation_index) {
    auto method_holder = create_inbounds_gep(function, { 0, method_index, specialisation_index });
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

void CodeGenerator::push_replacement_type_parameter(types::ParameterType *key, types::Type *value) {
    m_replacement_type_parameters[key] = value;
}

void CodeGenerator::pop_replacement_type_parameter(types::ParameterType *key) {
    m_replacement_type_parameters.erase(key);
}

void CodeGenerator::push_replacement_generic_specialisation(std::map<types::ParameterType *, types::Type *> specialisation) {
    for (auto const &entry : specialisation) {
        push_replacement_type_parameter(entry.first, entry.second);
    }
}

void CodeGenerator::pop_replacement_generic_specialisation(std::map<types::ParameterType *, types::Type *> specialisation) {
    for (auto const &entry : specialisation) {
        pop_replacement_type_parameter(entry.first);
    }
}

types::Type *CodeGenerator::get_replacement_type_parameter(types::ParameterType *key) {
    return m_replacement_type_parameters[key];
}

types::Type *CodeGenerator::get_replacement_type_parameter(types::Parameter *key) {
    return get_replacement_type_parameter(key->type());
}

void CodeGenerator::push_llvm_type_and_initialiser(llvm::Type *type, llvm::Constant *initialiser) {
    push_llvm_type(type);
    push_llvm_initialiser(initialiser);
}

void CodeGenerator::push_null_llvm_type_and_initialiser() {
    push_llvm_type_and_initialiser(nullptr, nullptr);
}

bool CodeGenerator::verify_function(ast::Node *node, llvm::Function *function) {
    std::string str;
    llvm::raw_string_ostream stream(str);
    if (llvm::verifyFunction(*function, &stream)) {
        function->dump();
        report(InternalError(node, stream.str()));
        return false;
    } else {
        return true;
    }
}

llvm::Function *CodeGenerator::create_function(llvm::Type *type, std::string name) const {
    return_null_if_null(type);

    auto function_type = llvm::cast<llvm::FunctionType>(type);
    return llvm::Function::Create(
        function_type, llvm::Function::ExternalLinkage,
        name, m_module
    );
}

llvm::GlobalVariable *CodeGenerator::create_global_variable(llvm::Type *type, llvm::Constant *initialiser, std::string name) {
    return_null_if_null(type);
    return_null_if_null(initialiser);

    auto variable = new llvm::GlobalVariable(
        *m_module, type, false,
        llvm::GlobalValue::InternalLinkage, initialiser, mangle(name)
    );

    variable->setAlignment(4);
    variable->setVisibility(
        llvm::GlobalValue::DefaultVisibility
    );

    return variable;
}

void CodeGenerator::prepare_method_parameters(ast::Def *node, llvm::Function *function) {
    auto name = static_cast<ast::Name *>(node->name());

    for (auto param : name->parameters()) {
        auto symbol = scope()->lookup(this, node, param->value());
        auto alloca = m_ir_builder->CreateAlloca(m_ir_builder->getInt1Ty(), 0, param->value());
        m_ir_builder->CreateStore(m_ir_builder->getInt1(false), alloca);
        symbol->value = alloca;
    }

    int i = 0;
    for (auto &arg : function->args()) {
        auto parameter = node->get_parameter(i);
        std::string arg_name = parameter->name()->value();
        arg.setName(arg_name);

        llvm::Value *value = &arg;

        if (!parameter->inout()) {
            auto alloca = m_ir_builder->CreateAlloca(arg.getType(), 0, arg_name);
            m_ir_builder->CreateStore(&arg, alloca);
            value = alloca;
        }

        auto symbol = scope()->lookup(this, node, arg_name);
        symbol->value = value;

        i++;
    }
}

llvm::Value *CodeGenerator::generate_builtin_variable(ast::VariableDeclaration *node) {
    if (node->name()->value() == "true") {
        return m_ir_builder->getInt1(1);
    } else {
        return m_ir_builder->getInt1(0);
    }
}

void CodeGenerator::generate_builtin_method_body(ast::Def *node, llvm::Function *function) {
    std::string name = static_cast<ast::Name *>(node->name())->value();

    if (name == "*") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateMul(a_value, b_value, "multiplication"));
    } else if (name == "+") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        if (a_value->getType()->isFloatingPointTy()) {
            push_llvm_value(m_ir_builder->CreateFAdd(a_value, b_value, "addition"));
        } else {
            push_llvm_value(m_ir_builder->CreateAdd(a_value, b_value, "addition"));
        }
    } else if (name == "-") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateSub(a_value, b_value, "subtraction"));
    } else if (name == "==") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateICmpEQ(a_value, b_value, "eq"));
    } else if (name == "!=") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateICmpNE(a_value, b_value, "neq"));
    } else if (name == "<") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateICmpSLT(a_value, b_value, "lt"));
    } else if (name == ">") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateICmpSGT(a_value, b_value, "gt"));
    } else if (name == ">=") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateICmpSGE(a_value, b_value, "gte"));
    } else if (name == "<=") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->value);
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->value);
        push_llvm_value(m_ir_builder->CreateICmpSLE(a_value, b_value, "lte"));
    } else if (name == "to_float") {
        auto value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "self")->value);
        push_llvm_value(m_ir_builder->CreateSIToFP(value, function->getReturnType(), "float"));
    } else if (name == "to_int") {
        auto value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "self")->value);
        push_llvm_value(m_ir_builder->CreateFPToSI(value, function->getReturnType(), "int"));
    } else {
        report(InternalError(node, "Unknown builtin definition."));
    }
}

llvm::Value *CodeGenerator::generate_llvm_value(ast::Node *node) {
    node->accept(this);
    auto value = pop_llvm_value();

    if (value == nullptr) {
        report(InternalError(node, "No LLVM value generated!"));
    }

    return value;
}

llvm::FunctionType *CodeGenerator::generate_function_type_for_method(types::Method *method) {
    auto llvm_return_type = generate_type(nullptr, method->return_type());
    return_null_if_null(llvm_return_type);

    std::vector<llvm::Type *> llvm_parameter_types;
    for (auto parameter_type : method->parameter_types()) {
        auto llvm_parameter_type = generate_type(nullptr, parameter_type);
        return_null_if_null(llvm_parameter_type);

        if (method->is_parameter_inout(parameter_type)) {
            llvm_parameter_type = llvm::PointerType::getUnqual(llvm_parameter_type);
        }

        llvm_parameter_types.push_back(llvm_parameter_type);
    }

    return llvm::FunctionType::get(
        llvm_return_type, llvm_parameter_types, false
    );
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
    auto it = m_replacement_type_parameters.find(type->type());
    if (it == m_replacement_type_parameters.end()) {
        push_null_llvm_type_and_initialiser();
    } else {
        it->second->accept(this);
    }
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
    std::vector<llvm::Type *> specialised_method_types;
    std::vector<llvm::Constant *> specialised_method_initialisers;

    for (auto specialisation : type->generic_specialisations()) {
        push_replacement_generic_specialisation(specialisation);
        auto llvm_type = generate_function_type_for_method(type);
        if (llvm_type == nullptr) {
            push_null_llvm_type_and_initialiser();
            return;
        }

        auto pointer_to_function = llvm::PointerType::getUnqual(llvm_type);
        specialised_method_types.push_back(pointer_to_function);
        specialised_method_initialisers.push_back(llvm::ConstantPointerNull::get(pointer_to_function));
        pop_replacement_generic_specialisation(specialisation);
    }

    auto struct_type = llvm::StructType::get(m_context, specialised_method_types);
    auto struct_initialiser = llvm::ConstantStruct::get(struct_type, specialised_method_initialisers);

    push_llvm_type_and_initialiser(struct_type, struct_initialiser);
}

void CodeGenerator::visit(types::Function *type) {
    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (int i = 0; i < type->no_methods(); i++) {
        auto method = type->get_method(i);
        type->set_llvm_index(method, i);

        auto llvm_type_for_method = generate_type(nullptr, method);
        auto llvm_initialiser_for_method = pop_llvm_initialiser();

        llvm_types.push_back(llvm_type_for_method);
        llvm_initialisers.push_back(llvm_initialiser_for_method);
    }

    auto llvm_type = llvm::StructType::get(m_context, llvm_types);
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

    push_insert_point();

    if (scope()->is_root()) {
        auto llvm_initialiser = take_initialiser(node);
        return_and_push_null_if_null(llvm_initialiser);

        auto variable = create_global_variable(llvm_type, llvm_initialiser, node->name()->value());
        symbol->value = variable;

        m_ir_builder->SetInsertPoint(&m_init_variables_function->getEntryBlock());
    } else {
        auto insert_function = m_ir_builder->GetInsertBlock()->getParent();
        m_ir_builder->SetInsertPoint(&insert_function->getEntryBlock().front());

        symbol->value = m_ir_builder->CreateAlloca(llvm_type, 0, node->name()->value());
    }

    pop_insert_point();

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
    auto llvm_method_index = function_type->get_llvm_index(method);
    auto llvm_specialisation_index = node->get_method_specialisation_index();

    auto ir_function = llvm::dyn_cast<llvm::LoadInst>(pop_llvm_value())->getPointerOperand();

    auto ir_method = m_ir_builder->CreateLoad(create_inbounds_gep(ir_function, { 0, llvm_method_index, llvm_specialisation_index }));

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

    auto call = m_ir_builder->CreateCall(ir_method, arguments);
    push_llvm_value(call);
}

void CodeGenerator::visit(ast::CCall *ccall) {
    auto return_type = generate_type(ccall);

    std::vector<llvm::Type *> parameters;
    for (auto parameter : ccall->parameters()) {
        parameters.push_back(generate_type(parameter));
    }

    auto llvm_function_type = llvm::FunctionType::get(
        return_type, parameters, false
    );

    auto llvm_function_name = ccall->name()->value();

    auto llvm_function = m_module->getOrInsertFunction(
        llvm_function_name, llvm_function_type
    );

    std::vector<llvm::Value *> arguments;
    for (auto argument : ccall->arguments()) {
        argument->accept(this);
        auto arg_value = pop_llvm_value();
        return_and_push_null_if_null(arg_value);
        arguments.push_back(arg_value);
    }

    push_llvm_value(m_ir_builder->CreateCall(llvm_function, arguments));
}

void CodeGenerator::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    auto value = pop_llvm_value();

    auto destination_type = generate_type(cast);
    auto new_value = m_ir_builder->CreateBitCast(value, destination_type);
    push_llvm_value(new_value);
}

void CodeGenerator::visit(ast::Assignment *node) {
    llvm::Value *rhs_value = nullptr;

    if (node->builtin()) {
        rhs_value = generate_builtin_variable(node->lhs);
    } else {
        rhs_value = generate_llvm_value(node->rhs);
    }

    return_if_null(rhs_value);

    node->lhs->accept(this);
    auto lhs_pointer = pop_llvm_value();

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

    expression->body().accept(this);
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

void CodeGenerator::visit(ast::Return *node) {
    auto value = generate_llvm_value(node->expression);
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

void CodeGenerator::visit(ast::Parameter *node) {
    report(InternalError(node, "N/A"));
}

void CodeGenerator::visit(ast::Let *node) {
    node->assignment->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
    }
}

void CodeGenerator::visit(ast::Def *node) {
    auto function_symbol = scope()->lookup(this, static_cast<ast::Name *>(node->name()));
    auto function_type = static_cast<types::Function *>(function_symbol->type);

    if (function_symbol->value == nullptr) {
        auto llvm_function_type = generate_type(node, function_type);
        auto llvm_initialiser = take_initialiser(node);
        function_symbol->value = create_global_variable(
            llvm_function_type, llvm_initialiser, static_cast<ast::Name *>(node->name())->value()
        );
    }

    auto method = static_cast<types::Method *>(node->type());

    auto symbol = function_symbol->nameSpace->lookup_by_node(this, node);

    auto llvm_function_name = codegen::mangle_method(function_symbol->name, method);

    auto llvm_method_type = llvm::cast<llvm::StructType>(generate_type(node));

    int specialisation_index = 0;
    for (auto specialisation : method->generic_specialisations()) {
        push_replacement_generic_specialisation(specialisation);

        push_scope(symbol);
        push_insert_point();

        auto llvm_specialised_method_type = llvm::cast<llvm::PointerType>(llvm_method_type->getElementType(specialisation_index))->getElementType();

        auto function = create_function(llvm_specialised_method_type, llvm_function_name);
        return_and_push_null_if_null(function);

        create_entry_basic_block(function, true);
        prepare_method_parameters(node, function);

        if (node->builtin()) {
            generate_builtin_method_body(node, function);
        } else {
            node->body()->accept(this);
        }

        auto value = pop_llvm_value();
        return_and_push_null_if_null(value);
        m_ir_builder->CreateRet(value);

        pop_insert_point();
        pop_scope();

        if (!verify_function(node, function)) {
            push_llvm_value(nullptr);
            return;
        }

        // FIXME return something better, like a load to the GEP pointer
        symbol->value = function;

        int llvm_method_index = function_type->get_llvm_index(method);
        create_store_method_to_function(function, function_symbol->value, llvm_method_index, specialisation_index);

        pop_replacement_generic_specialisation(specialisation);
        specialisation_index += 1;
    }

    push_llvm_value(symbol->value);
}

void CodeGenerator::visit(ast::Type *definition) {
    if (definition->builtin()) {
        // FIXME create a type thingy
        push_llvm_value(nullptr);
        return;
    }

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
        auto llvm_initialiser = take_initialiser(definition);

        // variable to hold the type
        auto variable = create_global_variable(llvm_type, llvm_initialiser, definition->name()->value());
        return_and_push_null_if_null(variable);

        symbol->value = variable;

        // create constructor function
        auto function_type = node_type->constructor();
        auto method_type = function_type->get_method(0);

        std::string mangled_name = codegen::mangle_method(definition->name()->value(), method_type);

        auto llvm_method_type = llvm::cast<llvm::StructType>(generate_type(definition, method_type));
        auto llvm_specialised_method_type = llvm::cast<llvm::FunctionType>(llvm::cast<llvm::PointerType>(llvm_method_type->getElementType(0))->getElementType());

        auto method = create_function(llvm_specialised_method_type, mangled_name);

        method->addFnAttr(llvm::Attribute::AlwaysInline);

        auto function = variable;
        create_store_method_to_function(method, function, 0, 0);

        push_insert_point();

        create_entry_basic_block(method, true);

        auto instance = m_ir_builder->CreateAlloca(llvm_specialised_method_type->getReturnType());

        int i = 0;
        for (auto &arg : method->getArgumentList()) {
            auto ptr = create_inbounds_gep(instance, { 0, i });
            m_ir_builder->CreateStore(&arg, ptr);
            i++;
        }

        m_ir_builder->CreateRet(m_ir_builder->CreateLoad(instance));

        pop_insert_point();

        push_llvm_value(variable);
    }
}

void CodeGenerator::visit(ast::Module *module) {
    auto symbol = scope()->lookup(this, module->name());
    return_and_push_null_if_null(symbol);

    push_scope(symbol);
    module->body().accept(this);
    pop_scope();
}

void CodeGenerator::visit(ast::Import *statement) {
    report(InternalError(statement, "N/A"));
}

void CodeGenerator::visit(ast::SourceFile *module) {
    m_module = new llvm::Module(module->name, m_context);

    auto void_function_type = llvm::FunctionType::get(m_ir_builder->getVoidTy(), false);

    m_init_variables_function = create_function(void_function_type, "_init_variables_");
    auto init_variables_bb = create_entry_basic_block(m_init_variables_function);

    auto user_code_function = create_function(void_function_type, "_user_code_");
    auto user_code_bb = create_entry_basic_block(user_code_function);

    auto int32_function_type = llvm::FunctionType::get(m_ir_builder->getInt32Ty(), false);
    auto main_function = create_function(int32_function_type, "main");
    auto main_bb = create_entry_basic_block(main_function);

    m_ir_builder->SetInsertPoint(user_code_bb);
    module->code->accept(this);
    m_ir_builder->CreateRetVoid();

    m_ir_builder->SetInsertPoint(main_bb);
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
