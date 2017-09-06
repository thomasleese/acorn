//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "acorn/ast/nodes.h"
#include "acorn/typesystem/types.h"

#include "acorn/prettyprinter.h"

using namespace acorn;

PrettyPrinter::PrettyPrinter() : m_indent(0) {

}

void PrettyPrinter::visit_node(ast::Node *node) {
    if (auto name = llvm::dyn_cast<ast::Name>(node)) {
        visit_terminal(name);
    } else if (auto int_ = llvm::dyn_cast<ast::Int>(node)) {
        visit_terminal(int_);
    } else if (auto float_ = llvm::dyn_cast<ast::Float>(node)) {
        visit_terminal(float_);
    } else if (auto complex = llvm::dyn_cast<ast::Complex>(node)) {
        visit_complex(complex);
    } else if (auto str = llvm::dyn_cast<ast::String>(node)) {
        visit_terminal(str);
    } else {
        visit_non_terminal(node);
    }
}

void PrettyPrinter::visit_complex(ast::Complex *node) {
    m_ss << indentation() << "(Complex [" << node->type_name() << "]" << "<unknown>" << ")\n";
}

void PrettyPrinter::visit_non_terminal(ast::Node *node) {
    m_ss << indentation() << "(" << node->kind_string() << " [" << node->type_name() << "]\n";
    m_indent++;

    ast::Visitor::visit_node(node);

    m_indent--;
    m_ss << indentation() << ")\n";
}

std::string PrettyPrinter::indentation() {
    std::string s;
    for (int i = 0; i < m_indent; i++) {
        s += " ";
    }
    return s;
}

std::string PrettyPrinter::str() {
    return m_ss.str();
}

void PrettyPrinter::print() {
    std::cout << str() << std::endl;
}
