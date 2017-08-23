//
// Created by Thomas Leese on 23/07/2017.
//

#include <llvm/Support/Casting.h>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/utils.h"

#include "acorn/ast/visitor.h"

using std::unique_ptr;

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
    } else if (auto name = llvm::dyn_cast<Name>(node)) {
        return visit_name(name);
    } else if (auto var_decl = llvm::dyn_cast<VariableDeclaration>(node)) {
        return visit_variable_declaration(var_decl);
    } else if (auto int_ = llvm::dyn_cast<Int>(node)) {
        return visit_int(int_);
    } else if (auto float_ = llvm::dyn_cast<Float>(node)) {
        return visit_float(float_);
    } else if (auto complex = llvm::dyn_cast<Complex>(node)) {
        return visit_complex(complex);
    } else if (auto str = llvm::dyn_cast<String>(node)) {
        return visit_string(str);
    } else if (auto list = llvm::dyn_cast<List>(node)) {
        return visit_list(list);
    } else if (auto tuple = llvm::dyn_cast<Tuple>(node)) {
        return visit_tuple(tuple);
    } else if (auto dict = llvm::dyn_cast<Dictionary>(node)) {
        return visit_dictionary(dict);
    } else if (auto call = llvm::dyn_cast<Call>(node)) {
        return visit_call(call);
    } else if (auto ccall = llvm::dyn_cast<CCall>(node)) {
        return visit_ccall(ccall);
    } else if (auto cast = llvm::dyn_cast<Cast>(node)) {
        return visit_cast(cast);
    } else if (auto assignment = llvm::dyn_cast<Assignment>(node)) {
        return visit_assignment(assignment);
    } else if (auto selector = llvm::dyn_cast<Selector>(node)) {
        return visit_selector(selector);
    } else if (auto while_ = llvm::dyn_cast<While>(node)) {
        return visit_while(while_);
    } else if (auto if_ = llvm::dyn_cast<If>(node)) {
        return visit_if(if_);
    } else if (auto return_ = llvm::dyn_cast<Return>(node)) {
        return visit_return(return_);
    } else if (auto spawn = llvm::dyn_cast<Spawn>(node)) {
        return visit_spawn(spawn);
    } else if (auto case_ = llvm::dyn_cast<Case>(node)) {
        return visit_case(case_);
    } else if (auto switch_ = llvm::dyn_cast<Switch>(node)) {
        return visit_switch(switch_);
    } else if (auto let = llvm::dyn_cast<Let>(node)) {
        return visit_let(let);
    } else if (auto parameter = llvm::dyn_cast<Parameter>(node)) {
        return visit_parameter(parameter);
    } else if (auto def = llvm::dyn_cast<Def>(node)) {
        return visit_def(def);
    } else if (auto type = llvm::dyn_cast<Type>(node)) {
        return visit_type(type);
    } else if (auto module = llvm::dyn_cast<Module>(node)) {
        return visit_module(module);
    } else if (auto import = llvm::dyn_cast<Import>(node)) {
        return visit_import(import);
    } else if (auto source_file = llvm::dyn_cast<SourceFile>(node)) {
        return visit_source_file(source_file);
    } else {
        logger->warn("Unknown node type: {}", node->token().to_string());
        return node;
    }
}

Node *Visitor::visit_block(Block *node) {
    auto &expressions = node->expressions();

    for (size_t i = 0; i < expressions.size(); i++) {
        expressions[i] = visit(expressions[i]);
    }

    return node;
}

Node *Visitor::visit_assignment(Assignment *node) {
    node->lhs() = unique_ptr<VariableDeclaration>(llvm::cast<VariableDeclaration>(visit(node->lhs()).release()));

    if (!node->builtin()) {
        node->rhs() = visit(node->rhs());
    }

    return node;
}

Node *Visitor::visit_while(While *node) {
    node->condition() = visit(node->condition());
    node->body() = visit(node->body());

    return node;
}

Node *Visitor::visit_case(Case *node) {
    node->condition() = visit(node->condition());
    node->assignment() = visit(node->assignment());
    node->body() = visit(node->body());

    return node;
}
