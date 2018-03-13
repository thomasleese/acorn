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
        void push_scope(Symbol &symbol);
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

        void visit_decl_name(ast::DeclName *node) override;

        void visit_var_decl(ast::VarDecl *node) override;
        void visit_parameter(ast::Parameter *node) override;
        void visit_def_decl(ast::DefDecl *node) override;
        void visit_type_decl(ast::TypeDecl *node) override;
        void visit_module_decl(ast::ModuleDecl *node) override;

    private:
        diagnostics::Logger m_logger;
        Namespace *m_root;
    };

}
