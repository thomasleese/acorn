//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef JET_SYMBOLTABLE_H
#define JET_SYMBOLTABLE_H

#include <map>
#include <string>
#include <vector>

#include "ast/visitor.h"
#include "compiler/pass.h"

namespace llvm {
    class Value;
}

namespace jet {

    namespace ast {
        struct Node;
    }

    namespace types {
        class Type;
    }

    namespace symboltable {

        struct Symbol;

        class Namespace {
        public:
            explicit Namespace(Namespace *parent);
            ~Namespace();

            bool has(std::string name, bool follow_parents = true) const;
            Symbol *lookup(ast::Node *currentNode, std::string name) const;
            Symbol *lookup(ast::Identifier *identifier) const;
            Symbol *lookup_by_node(ast::Node *node) const;
            void insert(ast::Node *currentNode, Symbol *symbol);
            void rename(Symbol *symbol, std::string new_name);
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

            void visit(ast::CodeBlock *block);
            void visit(ast::Identifier *identifier);
            void visit(ast::BooleanLiteral *boolean);
            void visit(ast::IntegerLiteral *expression);
            void visit(ast::FloatLiteral *expression);
            void visit(ast::ImaginaryLiteral *imaginary);
            void visit(ast::StringLiteral *expression);
            void visit(ast::SequenceLiteral *sequence);
            void visit(ast::MappingLiteral *mapping);
            void visit(ast::RecordLiteral *expression);
            void visit(ast::Argument *argument);
            void visit(ast::Call *expression);
            void visit(ast::CCall *expression);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::Index *expression);
            void visit(ast::Comma *expression);
            void visit(ast::While *expression);
            void visit(ast::For *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Sizeof *expression);
            void visit(ast::Strideof *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::VariableDefinition *definition);
            void visit(ast::FunctionDefinition *definition);
            void visit(ast::TypeDefinition *definition);
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

#endif // JET_SYMBOLTABLE_H
