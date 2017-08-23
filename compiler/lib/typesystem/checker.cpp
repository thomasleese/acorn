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

static auto logger = spdlog::get("acorn");

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::typesystem;

TypeChecker::TypeChecker(symboltable::Namespace *scope) {
    push_scope(scope);
}

void TypeChecker::check_types(ast::Expression *lhs, ast::Expression *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    if (!lhs->has_compatible_type_with(rhs)) {
        report(TypeMismatchError(rhs, lhs));
    }
}

void TypeChecker::check_not_null(ast::Expression *node) {
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

void TypeChecker::visit_name(ast::Name *node) {
    accept_many(node->parameters());
    check_not_null(node);
}

void TypeChecker::visit_variable_declaration(ast::VariableDeclaration *node) {
    visit(node->given_type());
    check_not_null(node);
}

void TypeChecker::visit_int(ast::Int *node) {
    check_not_null(node);
}

void TypeChecker::visit_float(ast::Float *node) {
    check_not_null(node);
}

void TypeChecker::visit_complex(ast::Complex *node) {
    check_not_null(node);
}

void TypeChecker::visit_string(ast::String *node) {
    check_not_null(node);
}

void TypeChecker::visit_list(ast::List *node) {
    accept_many(node->elements());
    check_not_null(node);
}

void TypeChecker::visit_dictionary(ast::Dictionary *node) {
    for (size_t i = 0; i < node->elements_size(); i++) {
        visit(node->key(i));
        visit(node->value(i));
    }

    check_not_null(node);
}

void TypeChecker::visit_tuple(ast::Tuple *node) {
    accept_many(node->elements());
    check_not_null(node);
}

void TypeChecker::visit_call(ast::Call *node) {
    visit(node->operand());
    accept_many(node->positional_arguments());

    for (auto &entry : node->keyword_arguments()) {
        visit(entry.second);
    }

    check_not_null(node);
}

void TypeChecker::visit_ccall(ast::CCall *node) {
    // node->name->accept(this);

    accept_many(node->parameters());
    accept(node->given_return_type());
    accept_many(node->arguments());

    check_not_null(node);
}

void TypeChecker::visit_cast(ast::Cast *node) {
    visit(node->operand());
    visit(node->new_type());

    check_not_null(node);
}

void TypeChecker::visit_assignment(ast::Assignment *node) {
    visit(node->lhs());

    if (!node->builtin()) {
        visit(node->rhs());
    }

    check_not_null(node);
}

void TypeChecker::visit_selector(ast::Selector *node) {
    accept_if_present(node->operand());

    check_not_null(node);
}

void TypeChecker::visit_while(ast::While *node) {
    visit(node->condition());
    visit(node->body());
    check_not_null(node);
}

void TypeChecker::visit_if(ast::If *node) {
    visit(node->condition());
    visit(node->true_case());
    accept_if_present(node->false_case());

    check_not_null(node);
}

void TypeChecker::visit_return(ast::Return *node) {
    visit(node->expression());
    check_not_null(node);
}

void TypeChecker::visit_spawn(ast::Spawn *node) {
    visit(node->call());
    check_not_null(node);
}

void TypeChecker::visit_switch(ast::Switch *node) {
    visit(node->expression());

    for (auto &entry : node->cases()) {
        visit(entry->condition());
        accept_if_present(entry->assignment());
        visit(entry->body());
        check_not_null(entry.get());
    }

    if (node->has_default_case()) {
        check_not_null(node->default_case());
    }

    check_not_null(node);
}

void TypeChecker::visit_parameter(ast::Parameter *node) {
    accept_if_present(node->given_type());
    check_not_null(node);
}

void TypeChecker::visit_let(ast::Let *node) {
    check_not_null(node);

    visit(node->assignment());

    if (node->has_body()) {
        visit(node->body());
        check_types(node, node->body());
    } else {
        check_types(node, node->assignment());
    }
}

void TypeChecker::visit_def(ast::Def *node) {
    check_not_null(node);

    auto name = node->name()->field();

    auto function_symbol = scope()->lookup(this, name);

    push_scope(function_symbol);

    auto method = static_cast<typesystem::Method *>(node->type());

    auto symbol = scope()->lookup(this, node, method->mangled_name());

    push_scope(symbol);

    accept(node->name());
    accept_many(name->parameters());

    if (node->builtin() || node->has_given_return_type()) {
        visit(node->given_return_type());
    }

    accept_many(node->parameters());

    if (!node->builtin()) {
        visit(node->body());
    }

    pop_scope();
    pop_scope();
}

void TypeChecker::visit_type(ast::Type *node) {
    check_not_null(node);

    accept(node->name());

    auto symbol = scope()->lookup(this, node->name());

    push_scope(symbol);

    if (node->has_alias()) {
        accept(node->alias());
    } else {
        /*for (auto name : node->field_names) {
            name->accept(this);
        }*/

        accept_many(node->field_types());
    }

    pop_scope();
}

void TypeChecker::visit_module(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name());
    return_if_null(symbol);

    push_scope(symbol);

    visit(node->body());
    check_not_null(node);

    pop_scope();
}

void TypeChecker::visit_import(ast::Import *node) {
    visit(node->path());
    check_not_null(node);
}

void TypeChecker::visit_source_file(ast::SourceFile *node) {
    accept_many(node->imports());
    visit(node->code());

    check_not_null(node);
}
