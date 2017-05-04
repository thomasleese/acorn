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

    return it->second;
}

Symbol *Namespace::lookup(Reporter *diagnostics, ast::Name *name) const {
    return lookup(diagnostics, name, name->value());
}

Symbol *Namespace::lookup_by_node(Reporter *diagnostics, ast::Node *node) const {
    for (auto entry : m_symbols) {
        if (entry.second->node == node) {
            return entry.second;
        }
    }

    if (m_parent) {
        return m_parent->lookup_by_node(diagnostics, node);
    } else {
        diagnostics->report(UndefinedError(node, node->token().lexeme));
        return nullptr;
    }
}

void Namespace::insert(Reporter *diagnostics, ast::Node *currentNode, Symbol *symbol) {
    symbol->node = currentNode;

    auto it = m_symbols.find(symbol->name);
    if (it != m_symbols.end()) {
        diagnostics->report(RedefinedError(currentNode, symbol->name));
    }

    m_symbols[symbol->name] = symbol;
}

void Namespace::rename(Reporter *diagnostics, Symbol *symbol, std::string new_name) {
    auto it = m_symbols.find(symbol->name);
    assert(it != m_symbols.end());

    m_symbols.erase(it);
    symbol->name = new_name;
    insert(diagnostics, symbol->node, symbol);
}

unsigned long Namespace::size() const {
    return m_symbols.size();
}

std::vector<Symbol *> Namespace::symbols() const {
    std::vector<Symbol *> symbols;
    for (auto entry : m_symbols) {
        symbols.push_back(entry.second);
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

    for (auto it = m_symbols.begin(); it != m_symbols.end(); it++) {
        auto symbol = it->second;
        ss << gap << " " << symbol->to_string(indent + 1) << "\n";
    }

    ss << gap << "}";

    return ss.str();
}

Symbol::Symbol(std::string name) : type(nullptr), value(nullptr), nameSpace(nullptr), node(nullptr), is_builtin(false) {
    this->name = name;
}

Symbol::Symbol(ast::Name *name) : Symbol(name->value()) {

}

bool Symbol::is_function() const {
    return dynamic_cast<types::Function *>(this->type) != nullptr && this->node == nullptr;
}

bool Symbol::is_type() const {
    return dynamic_cast<types::TypeType *>(this->type) != nullptr;
}

bool Symbol::is_variable() const {
    return dynamic_cast<ast::Let *>(this->node) != nullptr;
}

void Symbol::copy_type_from(ast::Expression *expression) {
    // TODO check type is not null?
    this->type = expression->type();
}

std::string Symbol::to_string(int indent) const {
    std::stringstream ss;
    ss << this->name << " (Node: " << this->node << ") (Value: " << this->value << ")";

    if (this->type) {
        ss << ": " << this->type->name();
    }

    if (this->nameSpace) {
        ss << " " << this->nameSpace->to_string(indent + 1);
    }

    return ss.str();
}

void ScopeFollower::push_scope(symboltable::Symbol *symbol) {
    push_scope(symbol->nameSpace);
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

Builder::Builder() {
    m_root = new Namespace(nullptr);
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

    Symbol *symbol = new Symbol(node->name()->value());
    symbol->is_builtin = node->builtin();
    scope()->insert(this, node, symbol);
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
    auto symbol = new Symbol(node->name()->value());
    scope()->insert(this, node, symbol);
}

void Builder::visit(ast::Let *node) {
    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
    }
}

void Builder::visit(ast::Def *node) {
    auto name = static_cast<ast::Name *>(node->name());

    Symbol *function_symbol;
    if (scope()->has(name->value(), false)) {
        // we don't want to look in any parent scope when we're
        // defining a new function; it should follow the notion of
        // variables, i.e. we are hiding the previous binding
        function_symbol = scope()->lookup(this, name);
    } else {
        function_symbol = new Symbol(name->value());
        function_symbol->nameSpace = new Namespace(scope());
        scope()->insert(this, node, function_symbol);
        function_symbol->node = nullptr;  // explicit no node for function symbols
    }

    // this is really hacky...
    auto pointer_location = reinterpret_cast<std::uintptr_t>(node);
    std::stringstream ss;
    ss << pointer_location;

    push_scope(function_symbol);

    auto symbol = new Symbol(ss.str());
    symbol->is_builtin = node->builtin();
    symbol->nameSpace = new Namespace(scope());
    scope()->insert(this, node, symbol);

    push_scope(symbol);

    for (auto parameter : name->parameters()) {
        auto sym = new Symbol(parameter->value());
        scope()->insert(this, parameter, sym);
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
    auto symbol = new Symbol(node->name()->value());
    symbol->is_builtin = node->builtin();
    scope()->insert(this, node, symbol);

    symbol->nameSpace = new Namespace(scope());

    push_scope(symbol);

    for (auto parameter : node->name()->parameters()) {
        auto sym = new Symbol(parameter->value());
        scope()->insert(this, parameter, sym);
    }

    if (node->has_alias()) {
        // do nothing
    } else {
        auto constructor_symbol = new Symbol("new");
        scope()->insert(this, node, constructor_symbol);
    }

    pop_scope();
}

void Builder::visit(ast::Module *node) {
    symboltable::Symbol *symbol;
    if (scope()->has(node->name()->value())) {
        symbol = scope()->lookup(this, node->name());
    } else {
        symbol = new Symbol(node->name());
        symbol->nameSpace = new Namespace(scope());
        scope()->insert(this, node, symbol);
    }

    push_scope(symbol);
    node->body()->accept(this);
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
