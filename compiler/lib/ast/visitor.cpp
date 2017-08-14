//
// Created by Thomas Leese on 23/07/2017.
//

#include "acorn/ast/nodes.h"
#include "acorn/utils.h"

#include "acorn/ast/visitor.h"

using namespace acorn::ast;

Visitor::~Visitor() {

}

void Visitor::accept(Node *node) {
    node->accept(this);
}

void Visitor::accept_if_present(Node *node) {
    return_if_null(node);
    accept(node);
}
