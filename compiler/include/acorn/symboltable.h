//
// Created by Thomas Leese on 15/03/2016.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "ast/visitor.h"
#include "diagnostics.h"

namespace llvm {
    class Value;
}

namespace acorn {

    namespace typesystem {
        class Type;
        class Method;
    }

}

namespace acorn::symboltable {

    class Symbol;

    class Namespace {
    public:
        explicit Namespace(Namespace *parent);
        ~Namespace();

        bool has(std::string name, bool follow_parents = true) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Node *current_node, std::string name) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Name *identifier) const;
        Symbol *lookup_by_node(diagnostics::Reporter *diagnostics, ast::Node *node) const;
        void insert(diagnostics::Reporter *diagnostics, ast::Node *current_node, std::unique_ptr<Symbol> symbol);
        void rename(diagnostics::Reporter *diagnostics, Symbol *symbol, std::string new_name);
        unsigned long size() const;
        std::vector<Symbol *> symbols() const;
        bool is_root() const;

        std::string to_string(int indent = 0) const;

    private:
        Namespace *m_parent;
        std::map<std::string, std::unique_ptr<Symbol>> m_symbols;
    };

    class Symbol {
    public:
        Symbol(std::string name, bool builtin);
        Symbol(ast::Name *name, bool builtin);

        std::string name() const { return m_name; }
        void set_name(std::string name) { m_name = name; }

        bool builtin() const { return m_builtin; }

        bool has_type() const { return m_type != nullptr; }
        typesystem::Type *type() const { return m_type; }
        void set_type(typesystem::Type *type) { m_type = type; }

        bool has_llvm_value() const { return m_llvm_value != nullptr; }
        llvm::Value *llvm_value() const { return m_llvm_value; }
        void set_llvm_value(llvm::Value *value) { m_llvm_value = value; }

        Namespace *scope() const { return m_scope.get(); }
        void initialise_scope(Namespace *parent);

        ast::Node *node() const { return m_node; }
        void initialise_node(ast::Node *node);

        bool is_function() const;
        bool is_variable() const;
        bool is_type() const;

        void copy_type_from(ast::Expression *expression);

        std::string to_string(int indent = 0) const;

    private:
        std::string m_name;
        bool m_builtin;
        typesystem::Type *m_type;
        llvm::Value *m_llvm_value;
        std::unique_ptr<Namespace> m_scope;
        ast::Node *m_node;
    };

    class ScopeFollower {

    public:
        void push_scope(symboltable::Symbol *symbol);
        void push_scope(symboltable::Namespace *name_space);
        void pop_scope();
        symboltable::Namespace *scope() const;

    private:
        std::vector<symboltable::Namespace *> m_scope;

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
