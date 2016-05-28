//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef ACORN_SYMBOLTABLE_H
#define ACORN_SYMBOLTABLE_H

#include <map>
#include <string>
#include <vector>

#include "ast/visitor.h"
#include "pass.h"

namespace llvm {
    class Value;
}

namespace acorn {

    namespace ast {
        struct Node;
    }

    namespace types {
        class Type;
        class Method;
    }

    namespace compiler {
        class Pass;
    }

    namespace symboltable {

        struct Symbol;

        class Namespace {
        public:
            explicit Namespace(Namespace *parent);
            ~Namespace();

            bool has(std::string name, bool follow_parents = true) const;
            Symbol *lookup(compiler::Pass *pass, ast::Node *currentNode, std::string name) const;
            Symbol *lookup(compiler::Pass *pass, ast::Identifier *identifier) const;
            Symbol *lookup_by_node(compiler::Pass *pass, ast::Node *node) const;
            void insert(compiler::Pass *pass, ast::Node *currentNode, Symbol *symbol);
            void rename(compiler::Pass *pass, Symbol *symbol, std::string new_name);
            unsigned long size() const;
            std::vector<Symbol *> symbols() const;
            bool is_root() const;

            std::string to_string() const;

            Namespace *clone() const;

        private:
            Namespace *m_parent;
            std::map<std::string, Symbol *> m_symbols;
        };

        struct Symbol {
            explicit Symbol(std::string name);

            std::string name;
            types::Type *type;
            llvm::Value *value;
            Namespace *nameSpace;
            ast::Node *node;
            bool is_builtin;

            bool is_function() const;
            bool is_variable() const;
            bool is_type() const;

            std::string to_string() const;

            Symbol *clone() const;
        };

        class Builder : public compiler::Pass, public ast::Visitor {
        public:
            explicit Builder();

            bool isAtRoot() const;
            Namespace *rootNamespace();

        private:
            Symbol *add_builtin_symbol(std::string name, types::Type *type);
            Symbol *add_builtin_function(std::string name);
            Symbol *add_builtin_method(symboltable::Symbol *function, types::Method *method);

            void add_builtin_types();
            void add_builtin_variables();
            void add_builtin_functions();

        public:
            void add_builtins();

            void visit(ast::CodeBlock *block);
            void visit(ast::Identifier *identifier);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::IntegerLiteral *expression);
            void visit(ast::FloatLiteral *expression);
            void visit(ast::ImaginaryLiteral *imaginary);
            void visit(ast::StringLiteral *expression);
            void visit(ast::SequenceLiteral *sequence);
            void visit(ast::MappingLiteral *mapping);
            void visit(ast::RecordLiteral *expression);
            void visit(ast::TupleLiteral *expression);
            void visit(ast::Call *expression);
            void visit(ast::CCall *expression);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::While *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Parameter *parameter);

            void visit(ast::VariableDefinition *definition);
            void visit(ast::FunctionDefinition *definition);
            void visit(ast::TypeDefinition *definition);
            void visit(ast::ProtocolDefinition *definition);
            void visit(ast::EnumDefinition *definition);

            void visit(ast::DefinitionStatement *statement);
            void visit(ast::ExpressionStatement *statement);
            void visit(ast::ImportStatement *statement);
            void visit(ast::SourceFile *module);

        private:
            Namespace *m_root;
            std::vector<Namespace *> m_scope;
        };

    };

}

#endif // ACORN_SYMBOLTABLE_H
