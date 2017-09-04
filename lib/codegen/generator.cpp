//
// Created by Thomas Leese on 12/01/2017.
//

#include <iostream>
#include <memory>
#include <sstream>

#include <spdlog/spdlog.h>

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

#include "acorn/ast/nodes.h"
#include "acorn/codegen/mangler.h"
#include "acorn/diagnostics.h"
#include "acorn/symboltable/namespace.h"
#include "acorn/symboltable/symbol.h"
#include "acorn/typesystem/types.h"
#include "acorn/utils.h"

#include "acorn/codegen/generator.h"

using namespace acorn;
using namespace acorn::codegen;
using namespace acorn::diagnostics;

static auto logger = spdlog::get("acorn");

CodeGenerator::CodeGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout) : IrBuilder(m_context), m_module(nullptr) {
    push_scope(scope);

    m_md_builder = std::make_unique<llvm::MDBuilder>(m_context);
    m_data_layout = data_layout;
}

llvm::Type *CodeGenerator::take_type() {
    if (has_llvm_type()) {
        auto result = pop_llvm_type();

        if (result == nullptr) {
            logger->critical("CodeGenerator::take_type pop_llvm_type returned nullptr");
            return nullptr;
        }

        return result;
    } else {
        logger->critical("CodeGenerator::take_type has_llvm_type was false");
        return nullptr;
    }
}

llvm::Constant *CodeGenerator::take_initialiser() {
    if (has_llvm_initialiser()) {
        auto result = pop_llvm_initialiser();

        if (result == nullptr) {
            logger->critical("CodeGenerator::take_initialiser pop_llvm_initialiser returned nullptr");
            return nullptr;
        }

        return result;
    } else {
        logger->critical("CodeGenerator::take_initialiser has_llvm_initialiser was false");
        return nullptr;
    }
}

llvm::Type *CodeGenerator::generate_type(typesystem::Type *type) {
    type->accept(this);
    return take_type();
}

llvm::Type *CodeGenerator::generate_type(ast::Node *node) {
    return generate_type(node->type());
}

void CodeGenerator::push_replacement_type_parameter(typesystem::ParameterType *key, typesystem::Type *value) {
    m_replacement_type_parameters[key] = value;
}

void CodeGenerator::pop_replacement_type_parameter(typesystem::ParameterType *key) {
    m_replacement_type_parameters.erase(key);
}

void CodeGenerator::push_replacement_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation) {
    for (auto const &entry : specialisation) {
        push_replacement_type_parameter(entry.first, entry.second);
    }
}

void CodeGenerator::pop_replacement_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation) {
    for (auto const &entry : specialisation) {
        pop_replacement_type_parameter(entry.first);
    }
}

typesystem::Type *CodeGenerator::get_replacement_type_parameter(typesystem::ParameterType *key) {
    return m_replacement_type_parameters[key];
}

typesystem::Type *CodeGenerator::get_replacement_type_parameter(typesystem::Parameter *key) {
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
        logger->critical(stream.str());
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
        name, module()
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

void CodeGenerator::prepare_method_parameters(ast::DefInstance *node, llvm::Function *function) {
    auto name = node->name()->field().get();

    for (auto &param : name->parameters()) {
        auto symbol = scope()->lookup(this, node, param->value());
        auto alloca = m_ir_builder->CreateAlloca(m_ir_builder->getInt1Ty(), 0, param->value());
        m_ir_builder->CreateStore(m_ir_builder->getInt1(false), alloca);
        symbol->set_llvm_value(alloca);
    }

    int i = 0;
    for (auto &arg : function->args()) {
        auto &parameter = node->parameters()[i];
        std::string arg_name = parameter->name()->value();
        arg.setName(arg_name);

        llvm::Value *value = &arg;

        if (!parameter->inout()) {
            auto alloca = m_ir_builder->CreateAlloca(arg.getType(), 0, arg_name);
            m_ir_builder->CreateStore(&arg, alloca);
            value = alloca;
        }

        auto symbol = scope()->lookup(this, node, arg_name);
        symbol->set_llvm_value(value);

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

void CodeGenerator::generate_builtin_method_body(ast::DefInstance *node, llvm::Function *function) {
    std::string name = node->name()->field()->value();

    if (name == "*") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateMul(a_value, b_value, "multiplication"));
    } else if (name == "+") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        if (a_value->getType()->isFloatingPointTy()) {
            push_llvm_value(m_ir_builder->CreateFAdd(a_value, b_value, "addition"));
        } else {
            push_llvm_value(m_ir_builder->CreateAdd(a_value, b_value, "addition"));
        }
    } else if (name == "-") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateSub(a_value, b_value, "subtraction"));
    } else if (name == "==") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateICmpEQ(a_value, b_value, "eq"));
    } else if (name == "!=") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateICmpNE(a_value, b_value, "neq"));
    } else if (name == "<") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateICmpSLT(a_value, b_value, "lt"));
    } else if (name == ">") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateICmpSGT(a_value, b_value, "gt"));
    } else if (name == ">=") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateICmpSGE(a_value, b_value, "gte"));
    } else if (name == "<=") {
        auto a_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "a")->llvm_value());
        auto b_value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "b")->llvm_value());
        push_llvm_value(m_ir_builder->CreateICmpSLE(a_value, b_value, "lte"));
    } else if (name == "to_float") {
        auto value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "self")->llvm_value());
        push_llvm_value(m_ir_builder->CreateSIToFP(value, function->getReturnType(), "float"));
    } else if (name == "to_int") {
        auto value = m_ir_builder->CreateLoad(scope()->lookup(this, node, "self")->llvm_value());
        push_llvm_value(m_ir_builder->CreateFPToSI(value, function->getReturnType(), "int"));
    } else {
        logger->critical("Unknown builtin definition.");
    }
}

llvm::Value *CodeGenerator::generate_llvm_value(ast::Node *node) {
    visit_node(node);
    auto value = pop_llvm_value();

    if (value == nullptr) {
        logger->critical("CodeGenerator::generate_llvm_value pop_llvm_value returned null");
    }

    return value;
}

llvm::FunctionType *CodeGenerator::generate_function_type_for_method(typesystem::Method *method) {
    auto llvm_return_type = generate_type(method->return_type());
    return_null_if_null(llvm_return_type);

    std::vector<llvm::Type *> llvm_parameter_types;
    for (auto parameter_type : method->parameter_types()) {
        auto llvm_parameter_type = generate_type(parameter_type);
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

void CodeGenerator::visit_constructor(typesystem::TypeType *type) {
    push_llvm_type_and_initialiser(
        m_ir_builder->getInt1Ty(),
        m_ir_builder->getInt1(0)
    );
}

void CodeGenerator::visit(typesystem::ParameterType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::VoidType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::BooleanType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::IntegerType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::UnsignedIntegerType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::FloatType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::UnsafePointerType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::FunctionType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::MethodType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::RecordType *type) {
    type->constructor()->accept(this);

    std::vector<llvm::Type *> struct_fields;
    struct_fields.push_back(pop_llvm_type());

    std::vector<llvm::Constant *> struct_initialisers;
    struct_initialisers.push_back(pop_llvm_initialiser());

    auto llvm_type = llvm::StructType::get(m_context, struct_fields);
    auto llvm_initialiser = llvm::ConstantStruct::get(llvm_type, struct_initialisers);

    push_llvm_type_and_initialiser(llvm_type, llvm_initialiser);
}

void CodeGenerator::visit(typesystem::TupleType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::AliasType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::ModuleType *type) {
    push_null_llvm_type_and_initialiser();
}

void CodeGenerator::visit(typesystem::TypeDescriptionType *type) {
    visit_constructor(type);
}

void CodeGenerator::visit(typesystem::Parameter *type) {
    auto it = m_replacement_type_parameters.find(type->type());
    if (it == m_replacement_type_parameters.end()) {
        push_null_llvm_type_and_initialiser();
    } else {
        it->second->accept(this);
    }
}

void CodeGenerator::visit(typesystem::Void *type) {
    push_llvm_type_and_initialiser(
        m_ir_builder->getInt1Ty(),
        m_ir_builder->getInt1(0)
    );
}

void CodeGenerator::visit(typesystem::Boolean *type) {
    push_llvm_type_and_initialiser(
        m_ir_builder->getInt1Ty(),
        m_ir_builder->getInt1(0)
    );
}

void CodeGenerator::visit(typesystem::Integer *type) {
    const unsigned int size = type->size();
    push_llvm_type_and_initialiser(
        m_ir_builder->getIntNTy(size),
        m_ir_builder->getIntN(size, 0)
    );
}

void CodeGenerator::visit(typesystem::UnsignedInteger *type) {
    const unsigned int size = type->size();
    push_llvm_type_and_initialiser(
        m_ir_builder->getIntNTy(size),
        m_ir_builder->getIntN(size, 0)
    );
}

void CodeGenerator::visit(typesystem::Float *type) {
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

void CodeGenerator::visit(typesystem::UnsafePointer *type) {
    auto llvm_element_type = generate_type(type->element_type());
    if (llvm_element_type == nullptr) {
        push_null_llvm_type_and_initialiser();
        return;
    }

    auto llvm_pointer_type = llvm::PointerType::getUnqual(llvm_element_type);
    push_llvm_type(llvm_pointer_type);
    push_llvm_initialiser(llvm::ConstantPointerNull::get(llvm_pointer_type));
}

void CodeGenerator::visit(typesystem::Record *type) {
    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (auto field_type : type->field_types()) {
        auto llvm_field_type = generate_type(field_type);
        auto llvm_field_initialiser = take_initialiser();

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

void CodeGenerator::visit(typesystem::Tuple *type) {
    visit(static_cast<typesystem::Record *>(type));
}

void CodeGenerator::visit(typesystem::Method *type) {
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

void CodeGenerator::visit(typesystem::Function *type) {
    std::vector<llvm::Type *> llvm_types;
    std::vector<llvm::Constant *> llvm_initialisers;

    for (int i = 0; i < type->no_methods(); i++) {
        auto method = type->get_method(i);
        type->set_llvm_index(method, i);

        auto llvm_type_for_method = generate_type(method);
        auto llvm_initialiser_for_method = pop_llvm_initialiser();

        llvm_types.push_back(llvm_type_for_method);
        llvm_initialisers.push_back(llvm_initialiser_for_method);
    }

    auto llvm_type = llvm::StructType::get(m_context, llvm_types);
    auto struct_initialiser = llvm::ConstantStruct::get(llvm_type, llvm_initialisers);

    push_llvm_type_and_initialiser(llvm_type, struct_initialiser);
}

void CodeGenerator::visit_block(ast::Block *node) {
    llvm::Value *last_value = nullptr;

    for (auto &expression : node->expressions()) {
        last_value = generate_llvm_value(expression.get());
        return_and_push_null_if_null(last_value);
    }

    push_llvm_value(last_value);
}

void CodeGenerator::visit_name(ast::Name *node) {
    auto symbol = scope()->lookup(this, node);
    return_and_push_null_if_null(symbol);

    if (!symbol->has_llvm_value()) {
        logger->critical("LLVM value is null.");
        push_llvm_value(nullptr);
        return;
    }

    push_llvm_value(m_ir_builder->CreateLoad(symbol->llvm_value()));
}

void CodeGenerator::visit_variable_declaration(ast::VariableDeclaration *node) {
    auto symbol = scope()->lookup(this, node->name().get());

    auto llvm_type = generate_type(node);
    return_and_push_null_if_null(llvm_type);

    push_insert_point();

    if (scope()->is_root()) {
        auto llvm_initialiser = take_initialiser();
        return_and_push_null_if_null(llvm_initialiser);

        auto variable = create_global_variable(llvm_type, llvm_initialiser, node->name()->value());
        symbol->set_llvm_value(variable);

        m_ir_builder->SetInsertPoint(&m_init_variables_function->getEntryBlock());
    } else {
        auto insert_function = m_ir_builder->GetInsertBlock()->getParent();
        m_ir_builder->SetInsertPoint(&insert_function->getEntryBlock().front());

        symbol->set_llvm_value(m_ir_builder->CreateAlloca(llvm_type, 0, node->name()->value()));
    }

    pop_insert_point();

    push_llvm_value(symbol->llvm_value());
}

void CodeGenerator::visit_int(ast::Int *node) {
    auto type = generate_type(node);

    uint64_t integer;
    std::stringstream ss;
    ss << node->value();
    ss >> integer;

    push_llvm_value(llvm::ConstantInt::get(type, integer, true));
}

void CodeGenerator::visit_float(ast::Float *node) {
    auto type = generate_type(node);

    double floatValue;
    std::stringstream ss;
    ss << node->value();
    ss >> floatValue;

    push_llvm_value(llvm::ConstantFP::get(type, floatValue));
}

void CodeGenerator::visit_complex(ast::Complex *node) {
    logger->warn("Visit ast::Complex not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_string(ast::String *node) {
    logger->warn("Visit ast::String not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_list(ast::List *node) {
    std::vector<llvm::Value *> elements;

    for (auto &element : node->elements()) {
        auto value = generate_llvm_value(element.get());
        return_and_push_null_if_null(value);
        elements.push_back(value);
    }

    auto type = static_cast<llvm::StructType *>(generate_type(node));
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

void CodeGenerator::visit_tuple(ast::Tuple *node) {
    auto llvm_type = generate_type(node);

    auto instance = m_ir_builder->CreateAlloca(llvm_type);

    int i = 0;
    for (auto &element : node->elements()) {
        auto value = generate_llvm_value(element.get());
        return_if_null(value);

        auto ptr = m_ir_builder->CreateInBoundsGEP(instance, build_gep_index({ 0, static_cast<int>(i) }));
        m_ir_builder->CreateStore(value, ptr);

        i++;
    }

    push_llvm_value(m_ir_builder->CreateLoad(instance));
}

void CodeGenerator::visit_dictionary(ast::Dictionary *node) {
    logger->warn("Visit ast::Dictionary not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_call(ast::Call *node) {
    auto &operand = node->operand();

    visit_node(operand.get());

    auto function_type = dynamic_cast<typesystem::Function *>(operand->type());

    auto method_index = node->get_method_index();
    auto method = function_type->get_method(method_index);
    auto llvm_method_index = function_type->get_llvm_index(method);
    auto llvm_specialisation_index = node->get_method_specialisation_index();

    auto ir_function = llvm::dyn_cast<llvm::LoadInst>(pop_llvm_value())->getPointerOperand();

    auto ir_method = m_ir_builder->CreateLoad(create_inbounds_gep(ir_function, { 0, llvm_method_index, llvm_specialisation_index }));

    if (ir_method == nullptr) {
        logger->critical("No LLVM function was available!");
        push_llvm_value(nullptr);
        return;
    }

    std::vector<llvm::Value *> arguments;
    int i = 0;
    bool valid;
    for (auto argument : method->ordered_arguments(node, &valid)) {
        auto value = generate_llvm_value(argument);

        if (method->is_parameter_inout(method->parameter_types()[i])) {
            auto load = llvm::dyn_cast<llvm::LoadInst>(value);
            assert(load);

            value = load->getPointerOperand();
        }

        arguments.push_back(value);
        i++;
    }

    if (!valid) {
        logger->critical("Could not order arguments!");
        push_llvm_value(nullptr);
        return;
    }

    auto call = m_ir_builder->CreateCall(ir_method, arguments);
    push_llvm_value(call);
}

void CodeGenerator::visit_ccall(ast::CCall *node) {
    auto return_type = generate_type(node);

    std::vector<llvm::Type *> parameters;
    for (auto &parameter : node->parameters()) {
        parameters.push_back(generate_type(parameter.get()));
    }

    auto llvm_function_type = llvm::FunctionType::get(
        return_type, parameters, false
    );

    auto llvm_function_name = node->name()->value();

    auto llvm_function = m_module->getOrInsertFunction(
        llvm_function_name, llvm_function_type
    );

    std::vector<llvm::Value *> arguments;
    for (auto &argument : node->arguments()) {
        auto arg_value = generate_llvm_value(argument.get());
        return_and_push_null_if_null(arg_value);
        arguments.push_back(arg_value);
    }

    push_llvm_value(m_ir_builder->CreateCall(llvm_function, arguments));
}

void CodeGenerator::visit_cast(ast::Cast *node) {
    auto value = generate_llvm_value(node->operand());

    auto destination_type = generate_type(node);
    auto new_value = m_ir_builder->CreateBitCast(value, destination_type);
    push_llvm_value(new_value);
}

void CodeGenerator::visit_assignment(ast::Assignment *node) {
    llvm::Value *rhs_value = nullptr;

    if (node->builtin()) {
        rhs_value = generate_builtin_variable(node->lhs().get());
    } else {
        rhs_value = generate_llvm_value(node->rhs());
    }

    return_if_null(rhs_value);

    auto lhs_pointer = generate_llvm_value(node->lhs().get());
    return_if_null(lhs_pointer);

    m_ir_builder->CreateStore(rhs_value, lhs_pointer);
    push_llvm_value(rhs_value);
}

void CodeGenerator::visit_selector(ast::Selector *node) {
    auto operand = node->operand().get();

    auto module_type = dynamic_cast<typesystem::ModuleType *>(operand->type());
    auto record_type_type = dynamic_cast<typesystem::RecordType *>(operand->type());
    auto record_type = dynamic_cast<typesystem::Record *>(operand->type());

    if (module_type) {
        auto module_name = static_cast<ast::Name *>(operand);

        auto symbol = scope()->lookup(this, module_name);
        return_and_push_null_if_null(symbol);

        push_scope(symbol);
        visit_node(node->field().get());
        pop_scope();
    } else if (record_type_type) {
        auto instance = generate_llvm_value(operand);
        return_if_null(instance);

        if (node->field()->value() == "new") {
            push_llvm_value(instance);
        } else {
            report(UndefinedError(node, "unsupported selector"));
            push_llvm_value(nullptr);
        }
    } else if (record_type) {
        auto instance = generate_llvm_value(operand);
        return_if_null(instance);

        auto actual_thing = llvm::cast<llvm::LoadInst>(instance)->getPointerOperand();

        auto record = dynamic_cast<typesystem::Record *>(operand->type());

        int index = record->get_field_index(node->field()->value());
        auto value = m_ir_builder->CreateLoad(m_ir_builder->CreateInBoundsGEP(actual_thing, build_gep_index({ 0, index })));
        push_llvm_value(value);
    } else {
        report(UndefinedError(node, "unsupported selector"));
        push_llvm_value(nullptr);
    }
}

void CodeGenerator::visit_while(ast::While *node) {
    auto entry_bb = create_basic_block("while_entry");
    auto loop_bb = create_basic_block("while_loop");
    auto join_bb = create_basic_block("while_join");

    m_ir_builder->CreateBr(entry_bb);

    m_ir_builder->SetInsertPoint(entry_bb);
    auto condition_value = generate_llvm_value(node->condition());
    auto condition = m_ir_builder->CreateICmpEQ(condition_value, m_ir_builder->getInt1(true), "while_cond");
    m_ir_builder->CreateCondBr(condition, loop_bb, join_bb);

    m_ir_builder->SetInsertPoint(loop_bb);

    auto then_value = generate_llvm_value(node->body());
    push_llvm_value(then_value);
    m_ir_builder->CreateBr(entry_bb);

    m_ir_builder->SetInsertPoint(join_bb);
}

void CodeGenerator::visit_if(ast::If *node) {
    auto condition = generate_llvm_value(node->condition());
    return_and_push_null_if_null(condition);

    condition = m_ir_builder->CreateICmpEQ(condition, m_ir_builder->getTrue(), "if_cond");

    auto then_bb = create_basic_block("if_then");
    auto else_bb = create_basic_block("if_else");
    auto join_bb = create_basic_block("if_join");

    m_ir_builder->CreateCondBr(condition, then_bb, else_bb);
    m_ir_builder->SetInsertPoint(then_bb);

    auto then_value = generate_llvm_value(node->true_case());
    return_and_push_null_if_null(then_value);
    m_ir_builder->CreateBr(join_bb);

    then_bb = m_ir_builder->GetInsertBlock();

    m_ir_builder->SetInsertPoint(else_bb);

    llvm::Value *else_value = nullptr;
    if (node->false_case()) {
        else_value = generate_llvm_value(node->false_case());
    } else {
        else_value = m_ir_builder->getInt1(false);
    }
    return_and_push_null_if_null(else_value);
    m_ir_builder->CreateBr(join_bb);

    else_bb = m_ir_builder->GetInsertBlock();

    m_ir_builder->SetInsertPoint(join_bb);

    auto type = generate_type(node);
    return_and_push_null_if_null(type);

    auto phi = m_ir_builder->CreatePHI(type, 2, "iftmp");
    phi->addIncoming(then_value, then_bb);
    phi->addIncoming(else_value, else_bb);

    push_llvm_value(phi);
}

void CodeGenerator::visit_return(ast::Return *node) {
    auto value = generate_llvm_value(node->expression());
    return_and_push_null_if_null(value);
    push_llvm_value(m_ir_builder->CreateRet(value));
}

void CodeGenerator::visit_spawn(ast::Spawn *node) {
    logger->warn("Visit ast::Spawn not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_switch(ast::Switch *node) {
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

    logger->warn("Visit ast::Switch not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_parameter(ast::Parameter *node) {
    logger->warn("Visit ast::Parameter not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_def_instance(ast::DefInstance *node) {
    auto function_symbol = scope()->lookup(this, node->name()->field().get());
    auto function_type = static_cast<typesystem::Function *>(function_symbol->type());

    if (!function_symbol->has_llvm_value()) {
        auto llvm_function_type = generate_type(function_type);
        auto llvm_initialiser = take_initialiser();

        function_symbol->set_llvm_value(
            create_global_variable(
                llvm_function_type, llvm_initialiser,
                node->name()->field()->value()
            )
        );
    }

    auto method = static_cast<typesystem::Method *>(node->type());

    auto symbol = function_symbol->scope()->lookup_by_node(this, node);

    auto llvm_function_name = codegen::mangle_method(function_symbol->name(), method);

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
            visit_node(node->body().get());
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
        symbol->set_llvm_value(function);

        int llvm_method_index = function_type->get_llvm_index(method);
        create_store_method_to_function(
            function, function_symbol->llvm_value(), llvm_method_index, specialisation_index
        );

        pop_replacement_generic_specialisation(specialisation);
        specialisation_index += 1;
    }

    push_llvm_value(symbol->llvm_value());
}

void CodeGenerator::visit_type_decl(ast::TypeDecl *node) {
    if (node->builtin()) {
        auto symbol = scope()->lookup(this, node->name().get());
        return_and_push_null_if_null(symbol);

        // FIXME create a proper type
        symbol->set_llvm_value(m_ir_builder->getInt1(0));
        push_llvm_value(symbol->llvm_value());
        return;
    }

    if (node->alias()) {
        auto new_symbol = scope()->lookup(this, node->name().get());
        auto old_symbol = scope()->lookup(this, node->alias().get(), node->alias()->value());
        new_symbol->set_llvm_value(old_symbol->llvm_value());
        push_llvm_value(new_symbol->llvm_value());
    } else {
        auto node_type = dynamic_cast<typesystem::RecordType *>(node->type());

        auto symbol = scope()->lookup(this, node->name().get());
        return_and_push_null_if_null(symbol);

        auto llvm_type = generate_type(node_type->constructor());
        auto llvm_initialiser = take_initialiser();

        // variable to hold the type
        auto variable = create_global_variable(llvm_type, llvm_initialiser, node->name()->value());
        return_and_push_null_if_null(variable);

        symbol->set_llvm_value(variable);

        // create constructor function
        auto function_type = node_type->constructor();
        auto method_type = function_type->get_method(0);

        std::string mangled_name = codegen::mangle_method(node->name()->value(), method_type);

        auto llvm_method_type = llvm::cast<llvm::StructType>(generate_type(method_type));

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

void CodeGenerator::visit_module(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name().get());
    return_and_push_null_if_null(symbol);

    push_scope(symbol);
    ast::Visitor::visit_module(node);
    pop_scope();
}

void CodeGenerator::visit_import(ast::Import *node) {
    logger->warn("Visit ast::Import not yet implemented.");
    push_llvm_value(nullptr);
}

void CodeGenerator::visit_source_file(ast::SourceFile *node) {
    if (!m_module) {
        m_module = std::make_unique<llvm::Module>(node->name(), m_context);

        auto void_function_type = llvm::FunctionType::get(m_ir_builder->getVoidTy(), false);

        m_init_variables_function = create_function(void_function_type, "_init_variables_");
        auto init_variables_bb = create_entry_basic_block(m_init_variables_function);

        auto user_code_function = create_function(void_function_type, "_user_code_");
        auto user_code_bb = create_entry_basic_block(user_code_function);

        auto int32_function_type = llvm::FunctionType::get(m_ir_builder->getInt32Ty(), false);
        auto main_function = create_function(int32_function_type, "main");
        auto main_bb = create_entry_basic_block(main_function);

        m_ir_builder->SetInsertPoint(user_code_bb);

        ast::Visitor::visit_source_file(node);

        m_ir_builder->CreateRetVoid();

        m_ir_builder->SetInsertPoint(main_bb);
        m_ir_builder->CreateCall(m_init_variables_function);
        m_ir_builder->CreateCall(user_code_function);
        m_ir_builder->CreateRet(m_ir_builder->getInt32(0));

        m_ir_builder->SetInsertPoint(init_variables_bb);
        m_ir_builder->CreateRetVoid();

        m_module->setDataLayout(*m_data_layout);

        std::string str;
        llvm::raw_string_ostream stream(str);
        if (llvm::verifyModule(*m_module, &stream)) {
            m_module->dump();
            logger->critical(stream.str());
        }
    } else {
        ast::Visitor::visit_source_file(node);
    }
}
