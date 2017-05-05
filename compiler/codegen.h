//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Module.h>

#include "ast.h"
#include "diagnostics.h"
#include "symboltable.h"
#include "types.h"

namespace llvm {
    class Module;
    class Function;
}

namespace acorn {

    namespace symboltable {
        class Namespace;
    }

    namespace codegen {

        std::string mangle(std::string name);
        std::string mangle_method(std::string name, types::Method *type);

        class ValueFollower {
        public:
            void push_llvm_value(llvm::Value *value);
            llvm::Value *pop_llvm_value();
            bool has_llvm_value() const;
            llvm::Value *llvm_value() const;

        private:
            std::vector<llvm::Value *> m_llvm_value_stack;
        };

        class TypeFollower {
        public:
            void push_llvm_type(llvm::Type *type);
            llvm::Type *pop_llvm_type();
            bool has_llvm_type() const;
            llvm::Type *llvm_type() const;

        private:
            std::vector<llvm::Type *> m_llvm_type_stack;
        };

        class InitialiserFollower {
        public:
            void push_llvm_initialiser(llvm::Constant *initialiser);
            llvm::Constant *pop_llvm_initialiser();
            bool has_llvm_initialiser() const;
            llvm::Constant *llvm_initialiser() const;

        private:
            std::vector<llvm::Constant *> m_llvm_initialiser_stack;
        };

        class IrBuilder {
        public:
            explicit IrBuilder(llvm::LLVMContext &context);

            void push_insert_point();
            void pop_insert_point();
            llvm::IRBuilderBase::InsertPoint insert_point() const;

            llvm::BasicBlock *create_basic_block(std::string name, llvm::Function *function = nullptr, bool set_insert_point = false);
            llvm::BasicBlock *create_entry_basic_block(llvm::Function *function = nullptr, bool set_insert_point = false);

            std::vector<llvm::Value *> build_gep_index(std::initializer_list<int> indexes);
            llvm::Value *create_inbounds_gep(llvm::Value *value, std::initializer_list<int> indexes);
            llvm::Value *create_store_method_to_function(llvm::Function *method, llvm::Value *function, int method_index, int specialisation_index);

        protected:
            llvm::IRBuilder<> *m_ir_builder;

        private:
            std::vector<llvm::IRBuilderBase::InsertPoint> m_insert_points;
        };

        class CodeGenerator : public ast::Visitor, public types::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower, public ValueFollower, public TypeFollower, public InitialiserFollower, public IrBuilder {

        public:
            CodeGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout);

            llvm::Module *module() const { return m_module.get(); }

            llvm::Type *take_type(ast::Expression *expression);
            llvm::Constant *take_initialiser(ast::Node *node);

            llvm::Type *generate_type(ast::Expression *expression, types::Type *type);
            llvm::Type *generate_type(ast::Expression *expression);

            void push_replacement_type_parameter(types::ParameterType *key, types::Type *value);
            void pop_replacement_type_parameter(types::ParameterType *key);
            void push_replacement_generic_specialisation(std::map<types::ParameterType *, types::Type *> specialisation);
            void pop_replacement_generic_specialisation(std::map<types::ParameterType *, types::Type *> specialisation);
            types::Type *get_replacement_type_parameter(types::ParameterType *key);
            types::Type *get_replacement_type_parameter(types::Parameter *key);

            void push_llvm_type_and_initialiser(llvm::Type *type, llvm::Constant *initialiser);
            void push_null_llvm_type_and_initialiser();

            bool verify_function(ast::Node *node, llvm::Function *function);

            llvm::Function *create_function(llvm::Type *type, std::string name) const;
            llvm::GlobalVariable *create_global_variable(llvm::Type *type, llvm::Constant *initialiser, std::string name);
            void prepare_method_parameters(ast::Def *node, llvm::Function *function);

            llvm::Value *generate_builtin_variable(ast::VariableDeclaration *node);
            void generate_builtin_method_body(ast::Def *node, llvm::Function *function);

            llvm::Value *generate_llvm_value(ast::Node *node);

            llvm::FunctionType *generate_function_type_for_method(types::Method *method);

            void visit_constructor(types::TypeType *type);

            void visit(types::ParameterType *type);
            void visit(types::VoidType *type);
            void visit(types::BooleanType *type);
            void visit(types::IntegerType *type);
            void visit(types::UnsignedIntegerType *type);
            void visit(types::FloatType *type);
            void visit(types::UnsafePointerType *type);
            void visit(types::FunctionType *type);
            void visit(types::MethodType *type);
            void visit(types::RecordType *type);
            void visit(types::TupleType *type);
            void visit(types::AliasType *type);
            void visit(types::ModuleType *type);
            void visit(types::ProtocolType *type);
            void visit(types::TypeDescriptionType *type);
            void visit(types::Parameter *type);
            void visit(types::Void *type);
            void visit(types::Boolean *type);
            void visit(types::Integer *type);
            void visit(types::UnsignedInteger *type);
            void visit(types::Float *type);
            void visit(types::UnsafePointer *type);
            void visit(types::Record *type);
            void visit(types::Tuple *type);
            void visit(types::Method *type);
            void visit(types::Function *type);

            void visit(ast::Block *node);
            void visit(ast::Name *node);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::Int *node);
            void visit(ast::Float *node);
            void visit(ast::Complex *node);
            void visit(ast::String *node);
            void visit(ast::List *node);
            void visit(ast::Dictionary *node);
            void visit(ast::Tuple *node);
            void visit(ast::Call *node);
            void visit(ast::CCall *node);
            void visit(ast::Cast *node);
            void visit(ast::Assignment *node);
            void visit(ast::Selector *node);
            void visit(ast::While *node);
            void visit(ast::If *node);
            void visit(ast::Return *node);
            void visit(ast::Spawn *node);
            void visit(ast::Switch *node);
            void visit(ast::Parameter *node);
            void visit(ast::Let *node);
            void visit(ast::Def *node);
            void visit(ast::Type *node);
            void visit(ast::Module *node);
            void visit(ast::Protocol *node);
            void visit(ast::Import *node);
            void visit(ast::SourceFile *node);

        private:
            llvm::LLVMContext m_context;
            std::unique_ptr<llvm::Module> m_module;
            std::unique_ptr<llvm::MDBuilder> m_md_builder;
            llvm::DataLayout *m_data_layout;

            std::vector<llvm::Argument *> m_args;
            std::map<types::ParameterType *, types::Type *> m_replacement_type_parameters;

            llvm::Function *m_init_variables_function;
        };

    }

}
