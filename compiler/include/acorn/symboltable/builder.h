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
        Namespace *m_root;
    };

}
