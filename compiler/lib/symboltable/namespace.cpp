//
// Created by Thomas Leese on 15/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/typesystem/types.h"

#include "acorn/symboltable/symbol.h"

#include "acorn/symboltable/namespace.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::symboltable;

Namespace::Namespace(Namespace *parent) : m_parent(parent) {

}

Namespace::~Namespace() {

}

bool Namespace::has(std::string name, bool follow_parents) const {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        if (follow_parents && m_parent) {
            return m_parent->has(name);
        } else {
            return false;
        }
    } else {
        return true;
    }
}

Symbol *Namespace::lookup(Reporter *diagnostics, ast::Node *current_node, std::string name) const {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        if (m_parent) {
            return m_parent->lookup(diagnostics, current_node, name);
        } else {
            diagnostics->report(UndefinedError(current_node, name));
            return nullptr;
        }
    }

    return it->second.get();
}

Symbol *Namespace::lookup(Reporter *diagnostics, ast::Name *name) const {
    return lookup(diagnostics, name, name->value());
}

Symbol *Namespace::lookup_by_node(Reporter *diagnostics, ast::Node *node) const {
    for (auto &entry : m_symbols) {
        auto &symbol = entry.second;
        if (symbol->node() == node) {
            return symbol.get();
        }
    }

    if (m_parent) {
        return m_parent->lookup_by_node(diagnostics, node);
    } else {
        diagnostics->report(UndefinedError(node, node->token().lexeme));
        return nullptr;
    }
}

void Namespace::insert(Reporter *diagnostics, ast::Node *current_node, std::unique_ptr<Symbol> symbol) {
    auto name = symbol->name();

    auto it = m_symbols.find(name);
    if (it != m_symbols.end()) {
        diagnostics->report(RedefinedError(current_node, name));
    }

    symbol->initialise_scope(this);
    symbol->initialise_node(current_node);

    m_symbols[name] = std::move(symbol);
}

void Namespace::rename(Reporter *diagnostics, Symbol *symbol, std::string new_name) {
    auto it = m_symbols.find(symbol->name());
    assert(it != m_symbols.end());
    it->second.release();

    m_symbols.erase(it);
    symbol->set_name(new_name);
    insert(diagnostics, symbol->node(), std::unique_ptr<Symbol>(symbol));
}

unsigned long Namespace::size() const {
    return m_symbols.size();
}

std::vector<Symbol *> Namespace::symbols() const {
    std::vector<Symbol *> symbols;
    for (auto &entry : m_symbols) {
        symbols.push_back(entry.second.get());
    }
    return symbols;
}

bool Namespace::is_root() const {
    return m_parent == nullptr;
}

std::string Namespace::to_string(int indent) const {
    std::stringstream ss;

    std::string gap = "";
    for (int i = 0; i < indent; i++) {
        gap += " ";
    }

    ss << gap << "{\n";

    for (auto &entry : m_symbols) {
        ss << gap << " " << entry.second->to_string(indent + 1) << "\n";
    }

    ss << gap << "}";

    return ss.str();
}
