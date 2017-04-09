//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef ACORN_PRETTYPRINTER_H
#define ACORN_PRETTYPRINTER_H

#include <sstream>
#include <string>

#include "ast.h"

namespace acorn {

    class PrettyPrinter : public ast::Visitor {

    public:
        PrettyPrinter();

    public:
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
        std::string indentation();

    public:
        std::string str();
        void print();

    private:
        int indent;
        std::stringstream ss;

    };

}

#endif // ACORN_PRETTYPRINTER_H
