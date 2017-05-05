//
// Created by Thomas Leese on 15/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "ast.h"
#include "diagnostics.h"
#include "lexer.h"
#include "types.h"

#include "symboltable.h"

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

Symbol::Symbol(std::string name, bool builtin) : m_name(name), m_builtin(builtin), m_type(nullptr), m_llvm_value(nullptr), m_scope(nullptr), m_node(nullptr) {

}

Symbol::Symbol(ast::Name *name, bool builtin) : Symbol(name->value(), builtin) {

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
    return dynamic_cast<types::Function *>(m_type) != nullptr;
}

bool Symbol::is_type() const {
    return dynamic_cast<types::TypeType *>(m_type) != nullptr;
}

bool Symbol::is_variable() const {
    return dynamic_cast<ast::Let *>(m_node) != nullptr;
}

void Symbol::copy_type_from(ast::Expression *expression) {
    // TODO check type is not null?
    set_type(expression->type());
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

void ScopeFollower::push_scope(symboltable::Symbol *symbol) {
    push_scope(symbol->scope());
}

void ScopeFollower::push_scope(symboltable::Namespace *name_space) {
    m_scope.push_back(name_space);
}

void ScopeFollower::pop_scope() {
    m_scope.pop_back();
}

symboltable::Namespace *ScopeFollower::scope() const {
    return m_scope.back();
}

Builder::Builder(Namespace *root_namespace) : m_root(root_namespace) {
    push_scope(m_root);
}

bool Builder::is_at_root() const {
    return m_root == scope();
}

Namespace *Builder::root_namespace() {
    return m_root;
}

void Builder::visit(ast::Block *node) {
    for (auto &expression : node->expressions()) {
        expression->accept(this);
    }
}

void Builder::visit(ast::Name *node) {

}

void Builder::visit(ast::VariableDeclaration *node) {
    assert(!node->name()->has_parameters());

    auto symbol = std::make_unique<Symbol>(node->name(), node->builtin());
    scope()->insert(this, node, std::move(symbol));
}

void Builder::visit(ast::Int *node) {

}

void Builder::visit(ast::Float *node) {

}

void Builder::visit(ast::Complex *node) {

}

void Builder::visit(ast::String *node) {

}

void Builder::visit(ast::List *node) {

}

void Builder::visit(ast::Dictionary *node) {

}

void Builder::visit(ast::Tuple *node) {

}

void Builder::visit(ast::Call *node) {

}

void Builder::visit(ast::CCall *node) {

}

void Builder::visit(ast::Cast *node) {

}

void Builder::visit(ast::Assignment *node) {
    node->lhs()->accept(this);

    if (!node->builtin()) {
        node->rhs()->accept(this);
    }
}

void Builder::visit(ast::Selector *node) {

}

void Builder::visit(ast::While *node) {
    node->condition()->accept(this);
    node->body()->accept(this);
}

void Builder::visit(ast::If *node) {
    node->condition()->accept(this);

    node->true_case()->accept(this);

    if (node->has_false_case()) {
        node->false_case()->accept(this);
    }
}

void Builder::visit(ast::Return *node) {

}

void Builder::visit(ast::Spawn *node) {

}

void Builder::visit(ast::Switch *node) {
    for (auto &entry : node->cases()) {
        entry->condition()->accept(this);

        if (entry->has_assignment()) {
            entry->assignment()->accept(this);
        }
    }

    if (node->has_default_case()) {
        node->default_case()->accept(this);
    }
}

void Builder::visit(ast::Parameter *node) {
    auto symbol = std::make_unique<Symbol>(node->name(), false);
    scope()->insert(this, node, std::move(symbol));
}

void Builder::visit(ast::Let *node) {
    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
    }
}

void Builder::visit(ast::Def *node) {
    auto name = node->name()->field();

    Symbol *function_symbol;
    if (scope()->has(name->value(), false)) {
        // we don't want to look in any parent scope when we're
        // defining a new function; it should follow the notion of
        // variables, i.e. we are hiding the previous binding
        function_symbol = scope()->lookup(this, name);
    } else {
        function_symbol = new Symbol(name, false);
        scope()->insert(this, nullptr, std::unique_ptr<Symbol>(function_symbol));
    }

    // this is really hacky...
    auto pointer_location = reinterpret_cast<std::uintptr_t>(node);
    std::stringstream ss;
    ss << pointer_location;

    push_scope(function_symbol);

    auto symbol = new Symbol(ss.str(), node->builtin());
    scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));

    push_scope(symbol);

    for (auto parameter : name->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter, false);
        scope()->insert(this, parameter, std::move(sym));
    }

    for (auto parameter : node->parameters()) {
        parameter->accept(this);
    }

    if (!node->builtin()) {
        node->body()->accept(this);
    }

    pop_scope();
    pop_scope();
}

void Builder::visit(ast::Type *node) {
    auto symbol = new Symbol(node->name(), node->builtin());
    scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));

    push_scope(symbol);

    for (auto parameter : node->name()->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter->value(), false);
        scope()->insert(this, parameter, std::move(sym));
    }

    if (node->has_alias()) {
        // do nothing
    } else {
        auto constructor_symbol = std::make_unique<Symbol>("new", true);
        scope()->insert(this, node, std::move(constructor_symbol));
    }

    pop_scope();
}

void Builder::visit(ast::Module *node) {
    symboltable::Symbol *symbol;
    if (scope()->has(node->name()->value())) {
        symbol = scope()->lookup(this, node->name());
    } else {
        symbol = new Symbol(node->name(), false);
        scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));
    }

    push_scope(symbol);
    node->body()->accept(this);
    pop_scope();
}

void Builder::visit(ast::Protocol *node) {
    auto symbol = new Symbol(node->name(), false);
    scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));

    push_scope(symbol);

    // FIXME do we need to do something here?

    pop_scope();
}

void Builder::visit(ast::Import *node) {

}

void Builder::visit(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);
}
