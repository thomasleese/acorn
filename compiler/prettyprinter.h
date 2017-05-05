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
        void visit(ast::Protocol *node);
        void visit(ast::Import *node);
        void visit(ast::SourceFile *node);

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
