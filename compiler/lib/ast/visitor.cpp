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
    node->accept(this);
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
        expressions[i] = std::unique_ptr<Expression>(llvm::cast<Expression>(visit(expression)));
    }

    return node;
}
