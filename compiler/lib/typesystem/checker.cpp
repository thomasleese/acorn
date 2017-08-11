//
// Created by Thomas Leese on 12/01/2017.
//

#include <cassert>
#include <iostream>
#include <set>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/typesystem/types.h"

#include "acorn/typesystem/checker.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::typesystem;

#define return_if_null(thing) if (thing == nullptr) return;
#define return_null_if_null(thing) if (thing == nullptr) return nullptr;
#define return_if_false(thing) if (thing == false) return;
#define return_if_null_type(node) return_if_false(node->has_type())

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

void TypeChecker::check_not_null(ast::Expression *expression) {
    if (!expression->has_type()) {
        report(InternalError(expression, "No type given for: " + Token::as_string(expression->token().kind)));
    }
}

void TypeChecker::visit(ast::Block *node) {
    for (auto &expression : node->expressions()) {
        expression->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Name *node) {
    for (auto &parameter : node->parameters()) {
        parameter->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::VariableDeclaration *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Int *node) {
    check_not_null(node);
}

void TypeChecker::visit(ast::Float *node) {
    check_not_null(node);
}

void TypeChecker::visit(ast::Complex *node) {
    check_not_null(node);
}

void TypeChecker::visit(ast::String *node) {
    check_not_null(node);
}

void TypeChecker::visit(ast::List *node) {
    for (auto element : node->elements()) {
        element->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Dictionary *node) {
    for (size_t i = 0; i < node->elements_size(); i++) {
        node->key(i)->accept(this);
        node->value(i)->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Tuple *node) {
    for (auto element : node->elements()) {
        element->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Call *node) {
    node->operand()->accept(this);

    for (auto &argument : node->positional_arguments()) {
        argument->accept(this);
    }

    for (auto &entry : node->keyword_arguments()) {
        entry.second->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::CCall *node) {
    // node->name->accept(this);

    for (auto param : node->parameters()) {
        param->accept(this);
    }

    node->given_return_type()->accept(this);

    for (auto argument : node->arguments()) {
        argument->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Cast *node) {
    node->operand()->accept(this);
    node->new_type()->accept(this);

    check_not_null(node);
}

void TypeChecker::visit(ast::Assignment *node) {
    node->lhs()->accept(this);

    if (!node->builtin()) {
        node->rhs()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Selector *node) {
    if (node->has_operand()) {
        node->operand()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::While *node) {
    node->condition()->accept(this);
    node->body()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit(ast::If *node) {
    node->condition()->accept(this);

    node->true_case()->accept(this);

    if (node->has_false_case()) {
        node->false_case()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Return *node) {
    node->expression()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit(ast::Spawn *node) {
    node->call()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit(ast::Switch *node) {
    node->expression()->accept(this);

    for (auto entry : node->cases()) {
        entry->condition()->accept(this);

        if (entry->has_assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);
        check_not_null(entry);
    }

    if (node->has_default_case()) {
        check_not_null(node->default_case());
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Parameter *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    check_not_null(node);
}

void TypeChecker::visit(ast::Let *node) {
    check_not_null(node);

    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
        check_types(node, node->body());
    } else {
        check_types(node, node->assignment());
    }
}

void TypeChecker::visit(ast::Def *node) {
    check_not_null(node);

    auto name = node->name()->field();

    auto function_symbol = scope()->lookup(this, name);

    push_scope(function_symbol);

    auto method = static_cast<typesystem::Method *>(node->type());

    auto symbol = scope()->lookup(this, node, method->mangled_name());

    push_scope(symbol);

    node->name()->accept(this);

    for (auto p : name->parameters()) {
        p->accept(this);
    }

    if (node->builtin() || node->has_given_return_type()) {
        node->given_return_type()->accept(this);
    }

    for (auto p : node->parameters()) {
        p->accept(this);
    }

    if (!node->builtin()) {
        node->body()->accept(this);
    }

    pop_scope();
    pop_scope();
}

void TypeChecker::visit(ast::Type *node) {
    check_not_null(node);

    node->name()->accept(this);

    auto symbol = scope()->lookup(this, node->name());

    push_scope(symbol);

    if (node->has_alias()) {
        node->alias()->accept(this);
    } else {
        /*for (auto name : node->field_names) {
            name->accept(this);
        }*/

        for (auto type : node->field_types()) {
            type->accept(this);
        }
    }

    pop_scope();
}

void TypeChecker::visit(ast::Module *node) {
    auto symbol = scope()->lookup(this, node->name());
    return_if_null(symbol);

    push_scope(symbol);

    node->body()->accept(this);
    check_not_null(node);

    pop_scope();
}

void TypeChecker::visit(ast::Import *node) {
    node->path()->accept(this);
    check_not_null(node);
}

void TypeChecker::visit(ast::SourceFile *node) {
    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);

    check_not_null(node);
}
