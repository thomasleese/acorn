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

        typesystem::TypeType *find_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
        typesystem::TypeType *find_type(ast::Node *node, std::string name);
        typesystem::TypeType *find_type(ast::Name *name);

        typesystem::Type *instance_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
        typesystem::Type *instance_type(ast::Node *node, std::string name);
        typesystem::Type *instance_type(ast::Name *name);

        typesystem::Type *builtin_type_from_name(ast::Name *node);

        bool infer_call_type_parameters(ast::Call *call, std::vector<typesystem::Type *> parameter_types,
                                        std::vector<typesystem::Type *> argument_types);

        typesystem::Type *replace_type_parameters(typesystem::Type *type,
                                                  std::map<typesystem::ParameterType *, typesystem::Type *> replacements);

    private:
        void check_types(ast::Node *lhs, ast::Node *rhs);
        void check_not_null(ast::Node *expression);

    public:
        ast::Node *visit_block(ast::Block *node) override;
        ast::Node *visit_name(ast::Name *node) override;
        ast::Node *visit_variable_declaration(ast::VariableDeclaration *node) override;
        ast::Node *visit_int(ast::Int *node) override;
        ast::Node *visit_float(ast::Float *node) override;
        ast::Node *visit_complex(ast::Complex *node) override;
        ast::Node *visit_string(ast::String *node) override;
        ast::Node *visit_list(ast::List *node) override;
        ast::Node *visit_tuple(ast::Tuple *node) override;
        ast::Node *visit_dictionary(ast::Dictionary *node) override;
        ast::Node *visit_call(ast::Call *node) override;
        ast::Node *visit_ccall(ast::CCall *node) override;
        ast::Node *visit_cast(ast::Cast *node) override;
        ast::Node *visit_assignment(ast::Assignment *node) override;
        ast::Node *visit_selector(ast::Selector *node) override;
        ast::Node *visit_while(ast::While *node) override;
        ast::Node *visit_if(ast::If *node) override;
        ast::Node *visit_return(ast::Return *node) override;
        ast::Node *visit_spawn(ast::Spawn *node) override;
        ast::Node *visit_case(ast::Case *node) override;
        ast::Node *visit_switch(ast::Switch *node) override;
        ast::Node *visit_parameter(ast::Parameter *node) override;
        ast::Node *visit_let(ast::Let *node) override;
        ast::Node *visit_def(ast::Def *node) override;
        ast::Node *visit_type(ast::Type *node) override;
        ast::Node *visit_module(ast::Module *node) override;
        ast::Node *visit_import(ast::Import *node) override;
        ast::Node *visit_source_file(ast::SourceFile *node) override;

    private:
        std::vector<ast::Def *> m_function_stack;

    };

}
