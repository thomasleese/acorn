//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <vector>

#include "../ast/visitor.h"
#include "../symboltable/builder.h"

namespace acorn {

    namespace diagnostics {
        class Reporter;
    }

    namespace typesystem {
        class Type;
        class TypeType;
        class ParameterType;
    }

}

namespace acorn::typesystem {

    class TypeChecker : public ast::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower {

    public:
        explicit TypeChecker(symboltable::Namespace *scope);

    private:
        typesystem::TypeType *find_type_constructor(ast::Node *node, std::string name);

        typesystem::TypeType *find_type(ast::Node *node, std::string name, std::vector<ast::TypeName *> parameters);
        typesystem::TypeType *find_type(ast::Node *node, std::string name);
        typesystem::TypeType *find_type(ast::TypeName *name);

        typesystem::Type *instance_type(ast::Node *node, std::string name, std::vector<ast::TypeName *> parameters);
        typesystem::Type *instance_type(ast::Node *node, std::string name);
        typesystem::Type *instance_type(ast::TypeName *name);

        typesystem::Type *builtin_type_from_name(ast::DeclName *node);

        bool infer_call_type_parameters(ast::Call *call, std::vector<typesystem::Type *> parameter_types,
                                        std::vector<typesystem::Type *> argument_types);

        typesystem::Type *replace_type_parameters(typesystem::Type *type,
                                                  std::map<typesystem::ParameterType *, typesystem::Type *> replacements);

    private:
        void check_types(ast::Node *lhs, ast::Node *rhs);
        void check_not_null(ast::Node *expression);

    public:
        void visit_block(ast::Block *node) override;
        void visit_name(ast::Name *node) override;
        void visit_type_name(ast::TypeName *node) override;
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
        void visit_case(ast::Case *node) override;
        void visit_switch(ast::Switch *node) override;
        void visit_parameter(ast::Parameter *node) override;
        void visit_let(ast::Let *node) override;
        void visit_def_decl(ast::DefDecl *node) override;
        void visit_type_decl(ast::TypeDecl *node) override;
        void visit_module_decl(ast::ModuleDecl *node) override;
        void visit_import(ast::Import *node) override;
        void visit_source_file(ast::SourceFile *node) override;

    private:
        std::vector<ast::DefDecl *> m_function_stack;

    };

}
