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

        ast::Node *visit_variable_declaration(ast::VariableDeclaration *node) override;
        ast::Node *visit_dictionary(ast::Dictionary *node) override;
        ast::Node *visit_call(ast::Call *node) override;
        ast::Node *visit_ccall(ast::CCall *node) override;
        ast::Node *visit_cast(ast::Cast *node) override;
        ast::Node *visit_selector(ast::Selector *node) override;
        ast::Node *visit_if(ast::If *node) override;
        ast::Node *visit_return(ast::Return *node) override;
        ast::Node *visit_spawn(ast::Spawn *node) override;
        ast::Node *visit_switch(ast::Switch *node) override;
        ast::Node *visit_parameter(ast::Parameter *node) override;
        ast::Node *visit_let(ast::Let *node) override;
        ast::Node *visit_def(ast::Def *node) override;
        ast::Node *visit_type(ast::Type *node) override;
        ast::Node *visit_module(ast::Module *node) override;
        ast::Node *visit_import(ast::Import *node) override;
        ast::Node *visit_source_file(ast::SourceFile *node) override;

    private:
        Namespace *m_root;
    };

}
