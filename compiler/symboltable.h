//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef ACORN_SYMBOLTABLE_H
#define ACORN_SYMBOLTABLE_H

#include <map>
#include <string>
#include <vector>

#include "ast.h"
#include "diagnostics.h"

namespace llvm {
    class Value;
}

namespace acorn {

    namespace types {
        class Type;
        class Method;
    }

    namespace symboltable {

        class Symbol;

        class Namespace {
        public:
            explicit Namespace(Namespace *parent);
            ~Namespace();

            bool has(std::string name, bool follow_parents = true) const;
            Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Node *current_node, std::string name) const;
            Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Name *identifier) const;
            Symbol *lookup_by_node(diagnostics::Reporter *diagnostics, ast::Node *node) const;
            void insert(diagnostics::Reporter *diagnostics, ast::Node *currentNode, Symbol *symbol);
            void rename(diagnostics::Reporter *diagnostics, Symbol *symbol, std::string new_name);
            unsigned long size() const;
            std::vector<Symbol *> symbols() const;
            bool is_root() const;

            std::string to_string(int indent = 0) const;

        private:
            Namespace *m_parent;
            std::map<std::string, Symbol *> m_symbols;
        };

        class Symbol {
        public:
            explicit Symbol(std::string name);
            explicit Symbol(ast::Name *name);

            std::string name;
            types::Type *type;
            llvm::Value *value;
            Namespace *nameSpace;
            ast::Node *node;
            bool is_builtin;

            bool is_function() const;
            bool is_variable() const;
            bool is_type() const;

            void copy_type_from(ast::Expression *expression);

            std::string to_string(int indent = 0) const;
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
            Builder();

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

    };

}

#endif // ACORN_SYMBOLTABLE_H
