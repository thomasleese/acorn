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

Node *Visitor::visit_name(Name *node) {
    auto &parameters = node->parameters();

    for (size_t i = 0; i < parameters.size(); i++) {
        parameters[i] = unique_ptr<Name>(llvm::cast<Name>(visit(parameters[i]).release()));
    }

    return node;
}

Node *Visitor::visit_variable_declaration(VariableDeclaration *node) {
    node->name() = unique_ptr<Name>(llvm::cast<Name>(visit(node->name()).release()));

    if (node->given_type()) {
        node->given_type() = unique_ptr<Name>(llvm::cast<Name>(visit(node->given_type()).release()));
    }

    return node;
}

Node *Visitor::visit_int(Int *node) {
    return node;
}

Node *Visitor::visit_float(Float *node) {
    return node;
}

Node *Visitor::visit_complex(Complex *node) {
    return node;
}

Node *Visitor::visit_string(String *node) {
    return node;
}

Node *Visitor::visit_list(List *node) {
    auto &elements = node->elements();

    for (size_t i = 0; i < elements.size(); i++) {
        elements[i] = visit(elements[i]);
    }

    return node;
}

Node *Visitor::visit_tuple(Tuple *node) {
    auto &elements = node->elements();

    for (size_t i = 0; i < elements.size(); i++) {
        elements[i] = visit(elements[i]);
    }

    return node;
}

Node *Visitor::visit_dictionary(Dictionary *node) {
    auto &keys = node->keys();
    auto &values = node->values();

    for (size_t i = 0; i < keys.size(); i++) {
        keys[i] = visit(keys[i]);
        values[i] = visit(values[i]);
    }

    return node;
}

Node *Visitor::visit_call(Call *node) {
    node->operand() = visit(node->operand());

    auto &positional_arguments = node->positional_arguments();

    for (size_t i = 0; i < positional_arguments.size(); i++) {
        positional_arguments[i] = visit(positional_arguments[i]);
    }

    for (auto &keyword_argument : node->keyword_arguments()) {
        keyword_argument.second = visit(keyword_argument.second);
    }

    return node;
}

Node *Visitor::visit_ccall(CCall *node) {
    auto &parameters = node->parameters();

    for (size_t i = 0; i < parameters.size(); i++) {
        parameters[i] = unique_ptr<Name>(llvm::cast<Name>(visit(parameters[i]).release()));
    }

    auto &arguments = node->arguments();

    for (size_t i = 0; i < arguments.size(); i++) {
        arguments[i] = visit(arguments[i]);
    }

    node->given_return_type() = unique_ptr<Name>(llvm::cast<Name>(visit(node->given_return_type()).release()));

    return node;
}

Node *Visitor::visit_cast(Cast *node) {
    node->operand() = visit(node->operand());

    node->new_type() = unique_ptr<Name>(llvm::cast<Name>(visit(node->operand()).release()));

    return node;
}

Node *Visitor::visit_assignment(Assignment *node) {
    node->lhs() = unique_ptr<VariableDeclaration>(llvm::cast<VariableDeclaration>(visit(node->lhs()).release()));

    if (!node->builtin()) {
        node->rhs() = visit(node->rhs());
    }

    return node;
}

Node *Visitor::visit_selector(Selector *node) {
    node->operand() = visit(node->operand());

    node->field() = unique_ptr<Name>(llvm::cast<Name>(visit(node->field()).release()));

    return node;
}

Node *Visitor::visit_while(While *node) {
    node->condition() = visit(node->condition());

    node->body() = visit(node->body());

    return node;
}

Node *Visitor::visit_if(If *node) {
    node->condition() = visit(node->condition());

    node->true_case() = visit(node->true_case());

    if (node->false_case()) {
        node->false_case() = visit(node->false_case());
    }

    return node;
}

Node *Visitor::visit_return(Return *node) {
    node->expression() = visit(node->expression());

    return node;
}

Node *Visitor::visit_spawn(Spawn *node) {
    node->call() = unique_ptr<Call>(llvm::cast<Call>(visit(node->call()).release()));

    return node;
}

Node *Visitor::visit_case(Case *node) {
    node->condition() = visit(node->condition());

    if (node->assignment()) {
        node->assignment() = visit(node->assignment());
    }

    node->body() = visit(node->body());

    return node;
}

Node *Visitor::visit_switch(Switch *node) {
    node->expression() = visit(node->expression());

    auto &cases = node->cases();

    for (size_t i = 0; i < cases.size(); i++) {
        cases[i] = unique_ptr<Case>(llvm::cast<Case>(visit(cases[i]).release()));
    }

    if (node->default_case()) {
        node->default_case() = visit(node->default_case());
    }

    return node;
}

Node *Visitor::visit_parameter(Parameter *node) {
    node->name() = unique_ptr<Name>(llvm::cast<Name>(visit(node->name()).release()));

    node->given_type() = unique_ptr<Name>(llvm::cast<Name>(visit(node->given_type()).release()));

    return node;
}

Node *Visitor::visit_let(Let *node) {
    node->assignment() = unique_ptr<Assignment>(llvm::cast<Assignment>(visit(node->assignment()).release()));

    if (node->body()) {
        node->body() = visit(node->body());
    }

    return node;
}

Node *Visitor::visit_def(Def *node) {
    node->name() = unique_ptr<Selector>(llvm::cast<Selector>(visit(node->name()).release()));

    auto &parameters = node->parameters();

    for (size_t i = 0; i < parameters.size(); i++) {
        parameters[i] = unique_ptr<Parameter>(llvm::cast<Parameter>(visit(parameters[i]).release()));
    }

    if (node->builtin() || node->given_return_type()) {
        node->given_return_type() = unique_ptr<Name>(llvm::cast<Name>(visit(node->given_return_type()).release()));
    }

    node->body() = visit(node->body());

    return node;
}

Node *Visitor::visit_type(Type *node) {
    node->name() = unique_ptr<Name>(llvm::cast<Name>(visit(node->name()).release()));

    if (node->alias()) {
        node->alias() = unique_ptr<Name>(llvm::cast<Name>(visit(node->alias()).release()));
    } else {
        auto &field_names = node->field_names();

        for (size_t i = 0; i < field_names.size(); i++) {
            field_names[i] = unique_ptr<Name>(llvm::cast<Name>(visit(field_names[i]).release()));
        }

        auto &field_types = node->field_types();

        for (size_t i = 0; i < field_types.size(); i++) {
            field_types[i] = unique_ptr<Name>(llvm::cast<Name>(visit(field_types[i]).release()));
        }
    }

    return node;
}

Node *Visitor::visit_module(Module *node) {
    node->name() = unique_ptr<Name>(llvm::cast<Name>(visit(node->name()).release()));

    node->body() = unique_ptr<Block>(llvm::cast<Block>(visit(node->body()).release()));

    return node;
}

Node *Visitor::visit_import(Import *node) {
    node->path() = unique_ptr<String>(llvm::cast<String>(visit(node->path()).release()));

    return node;
}

Node *Visitor::visit_source_file(SourceFile *node) {
    auto &imports = node->imports();

    for (size_t i = 0; i < imports.size(); i++) {
        imports[i] = unique_ptr<SourceFile>(llvm::cast<SourceFile>(visit(imports[i]).release()));
    }

    node->code() = unique_ptr<Block>(llvm::cast<Block>(visit(node->code()).release()));

    return node;
}
