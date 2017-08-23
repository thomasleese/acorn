//
// Created by Thomas Leese on 23/07/2017.
//

#include <llvm/Support/Casting.h>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/utils.h"

#include "acorn/ast/visitor.h"

using namespace acorn::ast;

static auto logger = spdlog::get("acorn");

Visitor::~Visitor() {

}

void Visitor::accept(Node *node) {
    visit(node);
}

void Visitor::accept_if_present(Node *node) {
    return_if_null(node);
    accept(node);
}

Node *Visitor::visit(Node *node) {
    if (auto block = llvm::dyn_cast<Block>(node)) {
        return visit_block(block);
    } else {
        logger->warn("Unknown node type: {}", node->token().to_string());
        return node;
    }
}

Node *Visitor::visit_block(Block *node) {
    auto &expressions = node->expressions();

    for (size_t i = 0; i < expressions.size(); i++) {
        auto expression = expressions[i].release();
        expressions[i] = std::unique_ptr<Node>(llvm::cast<Node>(visit(expression)));
    }

    return node;
}

void Visitor::visit_assignment(ast::Assignment *node) {
    visit(node->lhs());

    if (!node->builtin()) {
        visit(node->rhs());
    }
}

void Visitor::visit_while(ast::While *node) {
    visit(node->condition());
    visit(node->body());
}
