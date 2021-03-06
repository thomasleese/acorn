#pragma once

#include <string>

#include <llvm/IR/MDBuilder.h>

#include "../ast/visitor.h"
#include "../diagnostics.h"
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

        llvm::Type *generate_type(typesystem::Type *type);
        llvm::Type *generate_type(ast::Node *node);

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
        void prepare_method_parameters(ast::DefDecl *node, llvm::Function *function);

        llvm::Value *generate_builtin_variable(ast::VarDecl *node);
        void generate_builtin_method_body(ast::DefDecl *node, llvm::Function *function);

        llvm::Value *generate_llvm_value(ast::Node *node);

        llvm::Value *generate_llvm_value(std::unique_ptr<ast::Node> &node) {
            return generate_llvm_value(node.get());
        }

        llvm::FunctionType *generate_function_type_for_method(typesystem::Method *method);

        void visit_constructor(typesystem::TypeType *type);

        void visit(typesystem::ParameterType *type) override;
        void visit(typesystem::VoidType *type) override;
        void visit(typesystem::BooleanType *type) override;
        void visit(typesystem::IntegerType *type) override;
        void visit(typesystem::UnsignedIntegerType *type) override;
        void visit(typesystem::FloatType *type) override;
        void visit(typesystem::UnsafePointerType *type) override;
        void visit(typesystem::FunctionType *type) override;
        void visit(typesystem::MethodType *type) override;
        void visit(typesystem::RecordType *type) override;
        void visit(typesystem::TupleType *type) override;
        void visit(typesystem::AliasType *type) override;
        void visit(typesystem::ModuleType *type) override;
        void visit(typesystem::TypeDescriptionType *type) override;
        void visit(typesystem::Parameter *type) override;
        void visit(typesystem::Void *type) override;
        void visit(typesystem::Boolean *type) override;
        void visit(typesystem::Integer *type) override;
        void visit(typesystem::UnsignedInteger *type) override;
        void visit(typesystem::Float *type) override;
        void visit(typesystem::UnsafePointer *type) override;
        void visit(typesystem::Record *type) override;
        void visit(typesystem::Tuple *type) override;
        void visit(typesystem::Method *type) override;
        void visit(typesystem::Function *type) override;

        void visit_block(ast::Block *node) override;
        void visit_name(ast::Name *node) override;
        void visit_var_decl(ast::VarDecl *node) override;
        void visit_int(ast::Int *node) override;
        void visit_float(ast::Float *node) override;
        void visit_complex(ast::Complex *node) override;
        void visit_string(ast::String *node) override;
        void visit_list(ast::List *node) override;
        void visit_tuple(ast::Tuple *node) override;
        void visit_dictionary(ast::Dictionary *node) override;
        void visit_call(ast::Call *node) override;
        void visit_ccall(ast::CCall *node) override;
        void visit_cast(ast::Cast *node) override;
        void visit_assignment(ast::Assignment *node) override;
        void visit_selector(ast::Selector *node) override;
        void visit_while(ast::While *node) override;
        void visit_if(ast::If *node) override;
        void visit_return(ast::Return *node) override;
        void visit_spawn(ast::Spawn *node) override;
        void visit_switch(ast::Switch *node) override;
        void visit_parameter(ast::Parameter *node) override;
        void visit_def_decl(ast::DefDecl *node) override;
        void visit_type_decl(ast::TypeDecl *node) override;
        void visit_module_decl(ast::ModuleDecl *node) override;
        void visit_import(ast::Import *node) override;
        void visit_source_file(ast::SourceFile *node) override;

    private:
        diagnostics::Logger m_logger;

        llvm::LLVMContext m_context;
        std::unique_ptr<llvm::Module> m_module;
        std::unique_ptr<llvm::MDBuilder> m_md_builder;
        llvm::DataLayout *m_data_layout;

        std::vector<llvm::Argument *> m_args;
        std::map<typesystem::ParameterType *, typesystem::Type *> m_replacement_type_parameters;

        llvm::Function *m_init_variables_function;
    };

}
