//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef JET_SYMBOLTABLE_H
#define JET_SYMBOLTABLE_H

#include <map>
#include <string>

#include "AbstractSyntaxTree.h"

namespace llvm {
    class Value;
}

namespace Types {
    class Type;
}

namespace SymbolTable {

    struct Symbol;

    class Namespace {
    public:
        explicit Namespace(Namespace *parent);
        ~Namespace();

        bool has(std::string name, bool follow_parents = true) const;
        Symbol *lookup(AST::Node *currentNode, std::string name) const;
        Symbol *lookup(AST::Identifier *identifier) const;
        Symbol *lookup_by_node(AST::Node *node) const;
        void insert(AST::Node *currentNode, Symbol *symbol);
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
        Types::Type *type;
        llvm::Value *value;
        Namespace *nameSpace;
        AST::Node *node;

        bool is_function() const;

        std::string to_string() const;
        Symbol *clone() const;
    };

    class Builder : public AST::Visitor {
    public:
        explicit Builder();

        bool isAtRoot() const;
        Namespace *rootNamespace();

        void visit(AST::CodeBlock *block);

        void visit(AST::Identifier *identifier);
        void visit(AST::BooleanLiteral *boolean);
        void visit(AST::IntegerLiteral *expression);
        void visit(AST::FloatLiteral *expression);
        void visit(AST::ImaginaryLiteral *imaginary);
        void visit(AST::StringLiteral *expression);
        void visit(AST::SequenceLiteral *sequence);
        void visit(AST::MappingLiteral *mapping);
        void visit(AST::Argument *argument);
        void visit(AST::Call *expression);
        void visit(AST::CCall *expression);
        void visit(AST::Assignment *expression);
        void visit(AST::Selector *expression);
        void visit(AST::Index *expression);
        void visit(AST::Comma *expression);
        void visit(AST::While *expression);
        void visit(AST::For *expression);
        void visit(AST::If *expression);
        void visit(AST::Return *expression);
        void visit(AST::Spawn *expression);

        void visit(AST::Parameter *parameter);

        void visit(AST::VariableDefinition *definition);
        void visit(AST::FunctionDefinition *definition);
        void visit(AST::TypeDefinition *definition);

        void visit(AST::DefinitionStatement *statement);
        void visit(AST::ExpressionStatement *statement);
        void visit(AST::ImportStatement *statement);

        void visit(AST::SourceFile *module);

    private:
        Namespace *m_root;
        Namespace *m_current;
    };

};

#endif // JET_SYMBOLTABLE_H
