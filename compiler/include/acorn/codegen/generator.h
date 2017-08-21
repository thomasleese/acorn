//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <string>

#include <llvm/IR/MDBuilder.h>

#include "../ast/visitor.h"
#include "../symboltable/builder.h"
#include "../typesystem/visitor.h"

#include "followers.h"
#include "irbuilder.h"

namespace llvm {
    class Module;
    class Function;
    class MDBuilder;
}

namespace acorn {

    namespace symboltable {
        class Namespace;
    }

}

namespace acorn::codegen {

    class CodeGenerator : public ast::Visitor, public typesystem::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower, public ValueFollower, public TypeFollower, public InitialiserFollower, public IrBuilder {

    public:
        CodeGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout);

        llvm::Module *module() const { return m_module.get(); }

        llvm::Type *take_type();
        llvm::Constant *take_initialiser();

        llvm::Type *generate_type(ast::Expression *expression, typesystem::Type *type);
        llvm::Type *generate_type(ast::Expression *expression);

        void push_replacement_type_parameter(typesystem::ParameterType *key, typesystem::Type *value);
        void pop_replacement_type_parameter(typesystem::ParameterType *key);
        void push_replacement_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation);
        void pop_replacement_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation);
        typesystem::Type *get_replacement_type_parameter(typesystem::ParameterType *key);
        typesystem::Type *get_replacement_type_parameter(typesystem::Parameter *key);

        void push_llvm_type_and_initialiser(llvm::Type *type, llvm::Constant *initialiser);
        void push_null_llvm_type_and_initialiser();

        bool verify_function(ast::Node *node, llvm::Function *function);

        llvm::Function *create_function(llvm::Type *type, std::string name) const;
        llvm::GlobalVariable *create_global_variable(llvm::Type *type, llvm::Constant *initialiser, std::string name);
        void prepare_method_parameters(ast::Def *node, llvm::Function *function);

        llvm::Value *generate_builtin_variable(ast::VariableDeclaration *node);
        void generate_builtin_method_body(ast::Def *node, llvm::Function *function);

        llvm::Value *generate_llvm_value(ast::Node *node);

        llvm::FunctionType *generate_function_type_for_method(typesystem::Method *method);

        void visit_constructor(typesystem::TypeType *type);

        void visit(typesystem::ParameterType *type);
        void visit(typesystem::VoidType *type);
        void visit(typesystem::BooleanType *type);
        void visit(typesystem::IntegerType *type);
        void visit(typesystem::UnsignedIntegerType *type);
        void visit(typesystem::FloatType *type);
        void visit(typesystem::UnsafePointerType *type);
        void visit(typesystem::FunctionType *type);
        void visit(typesystem::MethodType *type);
        void visit(typesystem::RecordType *type);
        void visit(typesystem::TupleType *type);
        void visit(typesystem::AliasType *type);
        void visit(typesystem::ModuleType *type);
        void visit(typesystem::TypeDescriptionType *type);
        void visit(typesystem::Parameter *type);
        void visit(typesystem::Void *type);
        void visit(typesystem::Boolean *type);
        void visit(typesystem::Integer *type);
        void visit(typesystem::UnsignedInteger *type);
        void visit(typesystem::Float *type);
        void visit(typesystem::UnsafePointer *type);
        void visit(typesystem::Record *type);
        void visit(typesystem::Tuple *type);
        void visit(typesystem::Method *type);
        void visit(typesystem::Function *type);

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
        void visit(ast::Import *node);
        void visit(ast::SourceFile *node);

    private:
        llvm::LLVMContext m_context;
        std::unique_ptr<llvm::Module> m_module;
        std::unique_ptr<llvm::MDBuilder> m_md_builder;
        llvm::DataLayout *m_data_layout;

        std::vector<llvm::Argument *> m_args;
        std::map<typesystem::ParameterType *, typesystem::Type *> m_replacement_type_parameters;

        llvm::Function *m_init_variables_function;
    };

}
