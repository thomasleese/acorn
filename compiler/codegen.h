//
// Created by Thomas Leese on 12/01/2017.
//

#ifndef ACORN_CODEGEN_H
#define ACORN_CODEGEN_H

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
            llvm::Value *create_store_method_to_function(llvm::Function *method, llvm::Value *function, int index);

        protected:
            llvm::IRBuilder<> *m_ir_builder;

        private:
            std::vector<llvm::IRBuilderBase::InsertPoint> m_insert_points;
        };

        class CodeGenerator : public ast::Visitor, public types::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower, public ValueFollower, public TypeFollower, public InitialiserFollower, public IrBuilder {

        public:
            CodeGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout);
            ~CodeGenerator();

            llvm::Module *module() const;

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

        public:
            void builtin_generate();

        private:
            void builtin_initialise_boolean_variable(std::string name, bool value);

            void generate_builtin_method_body(ast::Def *node, llvm::Function *function);

        private:
            llvm::Value *generate_llvm_value(ast::Node *node);

        public:
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
            void visit(types::Module *type);

            void visit(ast::Block *block);
            void visit(ast::Name *expression);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::Int *expression);
            void visit(ast::Float *expression);
            void visit(ast::Complex *imaginary);
            void visit(ast::String *expression);
            void visit(ast::List *sequence);
            void visit(ast::Dictionary *mapping);
            void visit(ast::Tuple *expression);
            void visit(ast::Call *expression);
            void visit(ast::CCall *expression);
            void visit(ast::Cast *cast);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::While *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Switch *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::Let *definition);
            void visit(ast::Def *definition);
            void visit(ast::Type *definition);
            void visit(ast::Module *module);
            void visit(ast::Import *statement);
            void visit(ast::SourceFile *module);

        private:
            llvm::LLVMContext m_context;
            llvm::Module *m_module;
            llvm::MDBuilder *m_md_builder;
            llvm::DataLayout *m_data_layout;

            std::vector<llvm::Argument *> m_args;
            std::map<types::ParameterType *, types::Type *> m_replacement_type_parameters;

            llvm::Function *m_init_builtins_function;
            llvm::Function *m_init_variables_function;
        };

    }

}

#endif // ACORN_CODEGEN_H
