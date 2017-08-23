//
// Created by Thomas Leese on 14/03/2016.
//

#pragma once

#include <sstream>
#include <string>

#include "ast/visitor.h"

namespace acorn {

    class PrettyPrinter : public ast::Visitor {

    public:
        PrettyPrinter();

    public:
        ast::Node *visit_block(ast::Block *node) override;
        ast::Node *visit_name(ast::Name *node) override;
        ast::Node *visit_variable_declaration(ast::VariableDeclaration *node) override;
        ast::Node *visit_int(ast::Int *node) override;
        ast::Node *visit_float(ast::Float *node) override;
        ast::Node *visit_complex(ast::Complex *node) override;
        ast::Node *visit_string(ast::String *node) override;
        ast::Node *visit_list(ast::List *node) override;
        ast::Node *visit_dictionary(ast::Dictionary *node) override;
        ast::Node *visit_tuple(ast::Tuple *node) override;
        ast::Node *visit_call(ast::Call *node) override;
        ast::Node *visit_ccall(ast::CCall *node) override;
        ast::Node *visit_cast(ast::Cast *node) override;
        ast::Node *visit_assignment(ast::Assignment *node) override;
        ast::Node *visit_selector(ast::Selector *node) override;
        ast::Node *visit_while(ast::While *node) override;
        ast::Node *visit_if(ast::If *node) override;
        ast::Node *visit_return(ast::Return *node) override;
        ast::Node *visit_spawn(ast::Spawn *node) override;
        ast::Node *visit_switch(ast::Switch *node) override;
        ast::Node *visit_parameter(ast::Parameter *node) override;
        ast::Node *visit_let(ast::Let *node) override;
        ast::Node *visit_def(ast::Def *node) override;
        ast::Node *visit_type(ast::Type *node) override;
        ast::Node *visit_module(ast::Module *node) override;
        ast::Node *visit_import(ast::Import *node) override;
        ast::Node *visit_source_file(ast::SourceFile *node) override;

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
