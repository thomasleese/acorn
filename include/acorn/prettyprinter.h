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
        void visit_node(ast::Node *node) override;

        void visit_complex(ast::Complex *node) override;

        template<typename T>
        void visit_terminal(T *node) {
            m_ss << indentation() << "(" << node->kind_string() << " [" << node->type_name() << "] " << node->value() << ")\n";
        }

        void visit_non_terminal(ast::Node *node);

    private:
        std::string indentation();

    public:
        std::string str();
        void print();

    private:
        int m_indent;
        std::stringstream m_ss;

    };

}
