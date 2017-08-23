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

#include "acorn/symboltable/builder.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::symboltable;

static auto logger = spdlog::get("acorn");

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

ast::Node *Builder::visit_variable_declaration(ast::VariableDeclaration *node) {
    Visitor::visit_variable_declaration(node);

    if (node->name()->has_parameters()) {
        logger->warn("Builder::visit_variable_declaration name has parameters");
    }

    auto symbol = std::make_unique<Symbol>(node->name().get(), node->builtin());
    scope()->insert(this, node, std::move(symbol));

    return node;
}

ast::Node *Builder::visit_dictionary(ast::Dictionary *node) {
    return node;
}

ast::Node *Builder::visit_call(ast::Call *node) {
    return node;
}

ast::Node *Builder::visit_ccall(ast::CCall *node) {
    return node;
}

ast::Node *Builder::visit_cast(ast::Cast *node) {
    return node;
}

ast::Node *Builder::visit_selector(ast::Selector *node) {
    return node;
}

ast::Node *Builder::visit_if(ast::If *node) {
    visit(node->condition());
    visit(node->true_case());
    accept_if_present(node->false_case());
    return node;
}

ast::Node *Builder::visit_return(ast::Return *node) {
    return node;
}

ast::Node *Builder::visit_spawn(ast::Spawn *node) {
    return node;
}

ast::Node *Builder::visit_switch(ast::Switch *node) {
    for (auto &entry : node->cases()) {
        visit(entry->condition().get());
        accept_if_present(entry->assignment());
    }

    accept_if_present(node->default_case());

    return node;
}

ast::Node *Builder::visit_parameter(ast::Parameter *node) {
    auto symbol = std::make_unique<Symbol>(node->name(), false);
    scope()->insert(this, node, std::move(symbol));

    return node;
}

ast::Node *Builder::visit_let(ast::Let *node) {
    visit(node->assignment());
    accept_if_present(node->body());

    return node;
}

ast::Node *Builder::visit_def(ast::Def *node) {
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

    for (auto &parameter : name->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter.get(), false);
        scope()->insert(this, parameter.get(), std::move(sym));
    }

    accept_many(node->parameters());

    if (!node->builtin()) {
        visit(node->body());
    }

    pop_scope();
    pop_scope();

    return node;
}

ast::Node *Builder::visit_type(ast::Type *node) {
    auto symbol = new Symbol(node->name(), node->builtin());
    scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));

    push_scope(symbol);

    for (auto &parameter : node->name()->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter->value(), false);
        scope()->insert(this, parameter.get(), std::move(sym));
    }

    if (node->has_alias()) {
        // do nothing
    } else {
        auto constructor_symbol = std::make_unique<Symbol>("new", true);
        scope()->insert(this, node, std::move(constructor_symbol));
    }

    pop_scope();

    return node;
}

ast::Node *Builder::visit_module(ast::Module *node) {
    symboltable::Symbol *symbol;
    if (scope()->has(node->name()->value())) {
        symbol = scope()->lookup(this, node->name());
    } else {
        symbol = new Symbol(node->name(), false);
        scope()->insert(this, node, std::unique_ptr<Symbol>(symbol));
    }

    push_scope(symbol);
    visit(node->body());
    pop_scope();

    return node;
}

ast::Node *Builder::visit_import(ast::Import *node) {
    return node;
}

ast::Node *Builder::visit_source_file(ast::SourceFile *node) {
    accept_many(node->imports());
    visit(node->code());
    return node;
}
