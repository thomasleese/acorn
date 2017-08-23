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

void TypeChecker::visit_block(ast::Block *node) {
    for (auto &expression : node->expressions()) {
        expression->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_name(ast::Name *node) {
    for (auto &parameter : node->parameters()) {
        parameter->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_variable_declaration(ast::VariableDeclaration *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

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
        node->key(i)->accept(this);
        node->value(i)->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_tuple(ast::Tuple *node) {
    accept_many(node->elements());
    check_not_null(node);
}

void TypeChecker::visit_call(ast::Call *node) {
    node->operand()->accept(this);

    for (auto &argument : node->positional_arguments()) {
        argument->accept(this);
    }

    for (auto &entry : node->keyword_arguments()) {
        entry.second->accept(this);
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
    node->operand()->accept(this);
    node->new_type()->accept(this);

    check_not_null(node);
}

void TypeChecker::visit_assignment(ast::Assignment *node) {
    node->lhs()->accept(this);

    if (!node->builtin()) {
        node->rhs()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_selector(ast::Selector *node) {
    if (node->has_operand()) {
        node->operand()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_while(ast::While *node) {
    node->condition()->accept(this);
    node->body()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit_if(ast::If *node) {
    node->condition()->accept(this);

    node->true_case()->accept(this);

    if (node->has_false_case()) {
        node->false_case()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_return(ast::Return *node) {
    node->expression()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit_spawn(ast::Spawn *node) {
    node->call()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit_switch(ast::Switch *node) {
    node->expression()->accept(this);

    for (auto &entry : node->cases()) {
        entry->condition()->accept(this);

        if (entry->has_assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);
        check_not_null(entry.get());
    }

    if (node->has_default_case()) {
        check_not_null(node->default_case());
    }

    check_not_null(node);
}

void TypeChecker::visit_parameter(ast::Parameter *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit_let(ast::Let *node) {
    check_not_null(node);

    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
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
        node->given_return_type()->accept(this);
    }

    accept_many(node->parameters());

    if (!node->builtin()) {
        node->body()->accept(this);
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

    node->body()->accept(this);
    check_not_null(node);

    pop_scope();
}

void TypeChecker::visit_import(ast::Import *node) {
    node->path()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit_source_file(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);

    check_not_null(node);
}
