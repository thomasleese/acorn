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

        void visit_block(ast::Block *node) override;
        void visit_name(ast::Name *node) override;
        void visit_variable_declaration(ast::VariableDeclaration *node) override;
        void visit_int(ast::Int *node) override;
        void visit_float(ast::Float *node) override;
        void visit_complex(ast::Complex *node) override;
        void visit_string(ast::String *node) override;
        void visit_list(ast::List *node) override;
        void visit_dictionary(ast::Dictionary *node) override;
        void visit_tuple(ast::Tuple *node) override;
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
        void visit_let(ast::Let *node) override;
        void visit_def(ast::Def *node) override;
        void visit_type(ast::Type *node) override;
        void visit_module(ast::Module *node) override;
        void visit_import(ast::Import *node) override;
        void visit_source_file(ast::SourceFile *node) override;

    private:
        Namespace *m_root;
    };

}
