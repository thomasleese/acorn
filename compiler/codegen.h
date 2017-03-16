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

        class CodeGenerator : public ast::Visitor, public types::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower, public ValueFollower, public TypeFollower, public InitialiserFollower {

        public:
            CodeGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout);
            ~CodeGenerator();

            llvm::Module *module() const;

            llvm::Type *take_type(ast::Expression *expression);
            llvm::Constant *take_initialiser(ast::Node *node);

            void push_type_parameter(types::ParameterType *key, types::Type *value);
            void pop_type_parameter(types::ParameterType *key);
            types::Type *get_type_parameter(types::ParameterType *key);
            types::Type *get_type_parameter(types::Parameter *key);

            llvm::Type *generate_type(ast::Expression *expression, types::Type *type);
            llvm::Type *generate_type(ast::Expression *expression);

            void push_llvm_type_and_initialiser(llvm::Type *type, llvm::Constant *initialiser);
            void push_null_llvm_type_and_initialiser();

            llvm::Function *generate_function(ast::FunctionDefinition *definition);
            llvm::Function *generate_function(ast::FunctionDefinition *definition, std::map<types::ParameterType *, types::Type *>);

        public:
            void builtin_generate();

            llvm::Function *builtin_generate_function(std::string name, types::Method *method, std::string llvm_name);

        private:
            void builtin_generate_sizeof(types::Method *method, llvm::Function *function);
            void builtin_generate_strideof(types::Method *method, llvm::Function *function);

            llvm::Function *builtin_create_llvm_function(std::string name, int index);

            void builtin_initialise_boolean_variable(std::string name, bool value);
            void builtin_initialise_function(llvm::Function *function, int no_arguments);

        private:
            std::vector<llvm::Value *> build_gep_index(std::initializer_list<int> indexes);
            llvm::BasicBlock *create_basic_block(std::string name, llvm::Function *function = nullptr);
            llvm::BasicBlock *create_entry_basic_block(llvm::Function *function = nullptr);
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

            void visit(ast::Block *block);
            void visit(ast::Name *expression);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::IntegerLiteral *expression);
            void visit(ast::FloatLiteral *expression);
            void visit(ast::ImaginaryLiteral *imaginary);
            void visit(ast::StringLiteral *expression);
            void visit(ast::SequenceLiteral *sequence);
            void visit(ast::MappingLiteral *mapping);
            void visit(ast::RecordLiteral *expression);
            void visit(ast::TupleLiteral *expression);
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
            void visit(ast::VariableDefinition *definition);
            void visit(ast::FunctionDefinition *definition);
            void visit(ast::TypeDefinition *definition);
            void visit(ast::Import *statement);
            void visit(ast::SourceFile *module);

        private:
            llvm::LLVMContext m_context;
            llvm::Module *m_module;
            llvm::IRBuilder<> *m_ir_builder;
            llvm::MDBuilder *m_md_builder;
            llvm::DataLayout *m_data_layout;

            std::vector<llvm::Argument *> m_args;

            std::map<types::ParameterType *, types::Type *> m_type_parameters;

            llvm::Function *m_init_builtins_function;
            llvm::Function *m_init_variables_function;
        };

    }

}

#endif // ACORN_CODEGEN_H
