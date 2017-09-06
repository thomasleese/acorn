//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/typesystem/types.h"

#include "acorn/symboltable/namespace.h"

#include "acorn/symboltable/symbol.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::symboltable;

static auto logger = spdlog::get("acorn");

Symbol::Symbol(std::string name, bool builtin) : m_name(name), m_builtin(builtin), m_type(nullptr), m_llvm_value(nullptr), m_scope(nullptr), m_node(nullptr) {

}

Symbol::Symbol(ast::Name *name, bool builtin) : Symbol(name->value(), builtin) {

}

Symbol::Symbol(ast::TypeName *name, bool builtin) : Symbol(name->name().get(), builtin) {

}

Symbol::Symbol(ast::DeclName *name, bool builtin) : Symbol(name->name().get(), builtin) {

}

Symbol::Symbol(ast::ParamName *name, bool builtin) : Symbol(name->name().get(), builtin) {

}

void Symbol::initialise_scope(Namespace *parent) {
    if (m_scope) {
        assert(m_scope->parent() == parent);
    } else {
        m_scope = std::make_unique<Namespace>(parent);
    }
}

void Symbol::initialise_node(ast::Node *node) {
    assert(m_node == nullptr);
    m_node = node;
}

bool Symbol::is_function() const {
    return dynamic_cast<typesystem::Function *>(m_type) != nullptr;
}

bool Symbol::is_type() const {
    return dynamic_cast<typesystem::TypeType *>(m_type) != nullptr;
}

bool Symbol::is_variable() const {
    return dynamic_cast<ast::LetDecl *>(m_node) != nullptr;
}

void Symbol::copy_type_from(ast::Node *node) {
    if (node == nullptr || node->type() == nullptr) {
        logger->critical("Symbol::copy_type_from given a nullptr");
        return;
    }

    set_type(node->type());
}

std::string Symbol::to_string(int indent) const {
    std::stringstream ss;
    ss << m_name << " (Node: " << m_node << ") (LLVM Value: " << m_llvm_value << ")";

    if (m_type) {
        ss << ": " << m_type->name();
    }

    ss << " " << m_scope->to_string(indent + 1);

    return ss.str();
}
