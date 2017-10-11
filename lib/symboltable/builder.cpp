//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/symboltable/namespace.h"
#include "acorn/symboltable/symbol.h"
#include "acorn/typesystem/types.h"

#include "acorn/symboltable/builder.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::symboltable;

static auto logger = spdlog::get("acorn");

void ScopeFollower::push_scope(symboltable::Symbol *symbol) {
    push_scope(symbol->scope());
}

void ScopeFollower::push_scope(symboltable::Symbol &symbol) {
    push_scope(symbol.scope());
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

void Builder::visit_decl_name(ast::DeclName *node) {
    auto symbol = new Symbol(node->name(), false);
    scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));

    push_scope(symbol);

    for (auto &parameter : node->parameters()) {
        auto parameter_symbol = std::make_unique<Symbol>(parameter, false);
        scope()->insert(this, parameter, std::move(parameter_symbol));
    }

    pop_scope();
}

void Builder::visit_var_decl(ast::VarDecl *node) {
    Visitor::visit_var_decl(node);

    auto symbol = scope()->lookup(this, node->name());
    symbol->set_builtin(node->builtin());
}

void Builder::visit_parameter(ast::Parameter *node) {
    Visitor::visit_parameter(node);

    auto symbol = std::make_unique<Symbol>(node->name(), false);
    scope()->insert(this, node, std::move(symbol));
}

void Builder::visit_def_decl(ast::DefDecl *node) {
    auto name = node->name();

    Symbol *function_symbol;
    if (scope()->has(name->name()->value(), false)) {
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

    for (auto &parameter : name->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter, false);
        scope()->insert(this, parameter, std::move(sym));
    }

    for (auto &parameter : node->parameters()) {
        visit_node(parameter);
    }

    if (!node->builtin()) {
        visit_node(node->body().get());
    }

    pop_scope();
    pop_scope();
}

void Builder::visit_type_decl(ast::TypeDecl *node) {
    auto symbol = new Symbol(node->name(), node->builtin());
    scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));

    push_scope(symbol);

    for (auto &parameter : node->name()->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter->value(), false);
        scope()->insert(this, parameter, std::move(sym));
    }

    if (node->alias()) {
        // do nothing
    } else {
        auto constructor_symbol = std::make_unique<Symbol>("new", true);
        scope()->insert(this, node, std::move(constructor_symbol));
    }

    pop_scope();
}

void Builder::visit_module_decl(ast::ModuleDecl *node) {
    symboltable::Symbol *symbol;
    if (scope()->has(node->name()->name()->value())) {
        symbol = scope()->lookup(this, node->name());
    } else {
        symbol = new Symbol(node->name(), false);
        scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));
    }

    push_scope(symbol);
    visit_node(node->body().get());
    pop_scope();
}
