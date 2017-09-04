//
// Created by Thomas Leese on 15/03/2016.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "../ast/visitor.h"
#include "../diagnostics.h"

namespace acorn::symboltable {

    class Symbol;
    class Namespace;

    class ScopeFollower {

    public:
        void push_scope(Symbol *symbol);
        void push_scope(Namespace *name_space);
        void pop_scope();
        Namespace *scope() const;

    private:
        std::vector<Namespace *> m_scope;

    };

    class Builder : public ast::Visitor, public diagnostics::Reporter, ScopeFollower {
    public:
        explicit Builder(Namespace *root_namespace);

        bool is_at_root() const;
        Namespace *root_namespace();

        void visit_variable_declaration(ast::VariableDeclaration *node) override;
        void visit_parameter(ast::Parameter *node) override;
        void visit_def_instance(ast::DefInstance *node) override;
        void visit_type_decl(ast::TypeDecl *node) override;
        void visit_module(ast::Module *node) override;

    private:
        Namespace *m_root;
    };

}
