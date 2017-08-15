//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/typesystem/types.h"

#include "acorn/symboltable/namespace.h"
#include "acorn/symboltable/symbol.h"

#include "acorn/symboltable/builder.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::symboltable;

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

    for (auto &parameter : name->parameters()) {
        auto sym = std::make_unique<Symbol>(parameter.get(), false);
        scope()->insert(this, parameter.get(), std::move(sym));
    }

    accept_many(node->parameters());

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

void Builder::visit(ast::Import *node) {

}

void Builder::visit(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);
}