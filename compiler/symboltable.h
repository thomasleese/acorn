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
            Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Node *currentNode, std::string name) const;
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

        private:
            Symbol *add_builtin_symbol(std::string name, types::Type *type);
            Symbol *add_builtin_function(std::string name);
            Symbol *add_builtin_method(symboltable::Symbol *function, types::Method *method);

            void add_builtin_types();
            void add_builtin_variables();

        public:
            void add_builtins();

            void visit(ast::Block *block);
            void visit(ast::Name *identifier);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::Int *expression);
            void visit(ast::Float *expression);
            void visit(ast::Complex *imaginary);
            void visit(ast::String *expression);
            void visit(ast::List *sequence);
            void visit(ast::Dictionary *mapping);
            void visit(ast::Tuple *expression);
            void visit(ast::Call *expression);
            void visit(ast::CCall *expression);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::While *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Switch *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::Let *definition);
            void visit(ast::Def *definition);
            void visit(ast::Type *definition);
            void visit(ast::Module *module);
            void visit(ast::Import *statement);
            void visit(ast::SourceFile *module);

        private:
            Namespace *m_root;
        };

    };

}

#endif // ACORN_SYMBOLTABLE_H
