//
// Created by Thomas Leese on 15/03/2016.
//

#include "AbstractSyntaxTree.h"

#ifndef QUARK_SYMBOLTABLE_H
#define QUARK_SYMBOLTABLE_H

#include "AbstractSyntaxTree.h"

#include <string>

namespace SymbolTable {

    struct Symbol;

    class Namespace {
    public:
        explicit Namespace(Namespace *parent = 0);
        ~Namespace();

        Symbol *lookup(AST::Node *currentNode, std::string name) const;
        void insert(AST::Node *currentNode, Symbol *symbol);

    private:
        Namespace *m_parent;
        std::map<std::string, Symbol *> m_symbols;
    };

    struct Symbol {
        explicit Symbol(std::string name);

        std::string name;
        Namespace *nameSpace;
    };

    class Builder : public AST::Visitor {
    public:
        explicit Builder();

        bool isAtRoot() const;
        Namespace *rootNamespace();

        void visit(AST::CodeBlock *block);

        void visit(AST::Identifier *expression);
        void visit(AST::IntegerLiteral *expression);
        void visit(AST::FloatLiteral *expression);
        void visit(AST::StringLiteral *expression);
        void visit(AST::Argument *argument);
        void visit(AST::Call *expression);
        void visit(AST::Assignment *expression);
        void visit(AST::Selector *expression);
        void visit(AST::While *expression);
        void visit(AST::For *expression);
        void visit(AST::If *expression);

        void visit(AST::Type *type);
        void visit(AST::Parameter *parameter);

        void visit(AST::VariableDefinition *definition);
        void visit(AST::FunctionDefinition *definition);
        void visit(AST::TypeDefinition *definition);

        void visit(AST::DefinitionStatement *statement);
        void visit(AST::ExpressionStatement *statement);

        void visit(AST::Module *module);

    private:
        Namespace *m_root;
        Namespace *m_current;
    };

};

#endif //QUARK_SYMBOLTABLE_H
