//
// Created by Thomas Leese on 12/01/2017.
//

#include <iostream>
#include <set>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/symboltable/namespace.h"
#include "acorn/symboltable/symbol.h"
#include "acorn/typesystem/types.h"
#include "acorn/utils.h"

#include "acorn/typesystem/checker.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::typesystem;

static auto logger = spdlog::get("acorn");

TypeChecker::TypeChecker(symboltable::Namespace *scope) {
    push_scope(scope);
}

void TypeChecker::check_types(ast::Node *lhs, ast::Node *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    if (!lhs->has_compatible_type_with(rhs)) {
        report(TypeMismatchError(rhs, lhs));
    }
}

void TypeChecker::check_not_null(ast::Node *node) {
    if (!node->has_type()) {
        auto message = "No type given for: " + node->token().to_string();
        logger->critical(message);
    }
}

ast::Node *TypeChecker::visit_block(ast::Block *node) {
    ast::Visitor::visit_block(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_name(ast::Name *node) {
    ast::Visitor::visit_name(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_variable_declaration(ast::VariableDeclaration *node) {
    ast::Visitor::visit_variable_declaration(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_int(ast::Int *node) {
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_float(ast::Float *node) {
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_complex(ast::Complex *node) {
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_string(ast::String *node) {
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_list(ast::List *node) {
    ast::Visitor::visit_list(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_tuple(ast::Tuple *node) {
    ast::Visitor::visit_tuple(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_dictionary(ast::Dictionary *node) {
    ast::Visitor::visit_dictionary(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_call(ast::Call *node) {
    ast::Visitor::visit_call(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_ccall(ast::CCall *node) {
    ast::Visitor::visit_ccall(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_cast(ast::Cast *node) {
    ast::Visitor::visit_cast(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_assignment(ast::Assignment *node) {
    Visitor::visit_assignment(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_selector(ast::Selector *node) {
    Visitor::visit_selector(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_while(ast::While *node) {
    Visitor::visit_while(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_if(ast::If *node) {
    Visitor::visit_if(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_return(ast::Return *node) {
    Visitor::visit_return(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_spawn(ast::Spawn *node) {
    Visitor::visit_spawn(node);
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_switch(ast::Switch *node) {
    visit(node->expression().get());

    for (auto &entry : node->cases()) {
        visit(entry->condition().get());
        accept_if_present(entry->assignment());
        visit(entry->body().get());
        check_not_null(entry.get());
    }

    if (node->default_case()) {
        check_not_null(node->default_case().get());
    }

    check_not_null(node);

    return node;
}

ast::Node *TypeChecker::visit_parameter(ast::Parameter *node) {
    accept_if_present(node->given_type());
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_let(ast::Let *node) {
    check_not_null(node);

    visit(node->assignment().get());

    if (node->body()) {
        visit(node->body().get());
        check_types(node, node->body().get());
    } else {
        check_types(node, node->assignment().get());
    }

    return node;
}

ast::Node *TypeChecker::visit_def(ast::Def *node) {
    check_not_null(node);

    auto name = node->name()->field().get();

    auto function_symbol = scope()->lookup(this, name);

    push_scope(function_symbol);

    auto method = static_cast<typesystem::Method *>(node->type());

    auto symbol = scope()->lookup(this, node, method->mangled_name());

    push_scope(symbol);

    accept(node->name());
    accept_many(name->parameters());

    if (node->builtin() || node->given_return_type()) {
        visit(node->given_return_type().get());
    }

    accept_many(node->parameters());

    if (!node->builtin()) {
        visit(node->body().get());
    }

    pop_scope();
    pop_scope();

    return node;
}

ast::Node *TypeChecker::visit_type(ast::Type *node) {
    check_not_null(node);

    accept(node->name());

    auto symbol = scope()->lookup(this, node->name().get());

    push_scope(symbol);

    if (node->alias()) {
        accept(node->alias().get());
    } else {
        /*for (auto name : node->field_names) {
            name->accept(this);
        }*/

        accept_many(node->field_types());
    }

    pop_scope();

    return node;
}

ast::Node *TypeChecker::visit_module(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name().get());
    return_null_if_null(symbol);

    push_scope(symbol);

    visit(node->body().get());
    check_not_null(node);

    pop_scope();

    return node;
}

ast::Node *TypeChecker::visit_import(ast::Import *node) {
    visit(node->path().get());
    check_not_null(node);
    return node;
}

ast::Node *TypeChecker::visit_source_file(ast::SourceFile *node) {
    accept_many(node->imports());
    visit(node->code().get());
    check_not_null(node);
    return node;
}
