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

void Visitor::visit(Node *node) {
    if (node == nullptr) {
        logger->warn("Visitor::visit given a null node");
    }

    logger->debug("Visitor::visit {}", node->to_string());

    if (auto block = llvm::dyn_cast<Block>(node)) {
        visit_block(block);
    } else if (auto name = llvm::dyn_cast<Name>(node)) {
        visit_name(name);
    } else if (auto selector = llvm::dyn_cast<Selector>(node)) {
        visit_selector(selector);
    } else if (auto type_name = llvm::dyn_cast<TypeName>(node)) {
        visit_type_name(type_name);
    } else if (auto decl_name = llvm::dyn_cast<DeclName>(node)) {
        visit_decl_name(decl_name);
    } else if (auto var_decl = llvm::dyn_cast<VariableDeclaration>(node)) {
        visit_variable_declaration(var_decl);
    } else if (auto int_ = llvm::dyn_cast<Int>(node)) {
        visit_int(int_);
    } else if (auto float_ = llvm::dyn_cast<Float>(node)) {
        visit_float(float_);
    } else if (auto complex = llvm::dyn_cast<Complex>(node)) {
        visit_complex(complex);
    } else if (auto str = llvm::dyn_cast<String>(node)) {
        visit_string(str);
    } else if (auto list = llvm::dyn_cast<List>(node)) {
        visit_list(list);
    } else if (auto tuple = llvm::dyn_cast<Tuple>(node)) {
        visit_tuple(tuple);
    } else if (auto dict = llvm::dyn_cast<Dictionary>(node)) {
        visit_dictionary(dict);
    } else if (auto call = llvm::dyn_cast<Call>(node)) {
        visit_call(call);
    } else if (auto ccall = llvm::dyn_cast<CCall>(node)) {
        visit_ccall(ccall);
    } else if (auto cast = llvm::dyn_cast<Cast>(node)) {
        visit_cast(cast);
    } else if (auto assignment = llvm::dyn_cast<Assignment>(node)) {
        visit_assignment(assignment);
    } else if (auto while_ = llvm::dyn_cast<While>(node)) {
        visit_while(while_);
    } else if (auto if_ = llvm::dyn_cast<If>(node)) {
        visit_if(if_);
    } else if (auto return_ = llvm::dyn_cast<Return>(node)) {
        visit_return(return_);
    } else if (auto spawn = llvm::dyn_cast<Spawn>(node)) {
        visit_spawn(spawn);
    } else if (auto case_ = llvm::dyn_cast<Case>(node)) {
        visit_case(case_);
    } else if (auto switch_ = llvm::dyn_cast<Switch>(node)) {
        visit_switch(switch_);
    } else if (auto let = llvm::dyn_cast<Let>(node)) {
        visit_let(let);
    } else if (auto parameter = llvm::dyn_cast<Parameter>(node)) {
        visit_parameter(parameter);
    } else if (auto def = llvm::dyn_cast<Def>(node)) {
        visit_def(def);
    } else if (auto def_instance = llvm::dyn_cast<DefInstance>(node)) {
        visit_def_instance(def_instance);
    } else if (auto type_decl = llvm::dyn_cast<TypeDecl>(node)) {
        visit_type_decl(type_decl);
    } else if (auto module = llvm::dyn_cast<Module>(node)) {
        visit_module(module);
    } else if (auto import = llvm::dyn_cast<Import>(node)) {
        visit_import(import);
    } else if (auto source_file = llvm::dyn_cast<SourceFile>(node)) {
        visit_source_file(source_file);
    } else {
        logger->warn("Unknown node type: {}", node->token().to_string());
    }
}

void Visitor::visit_block(Block *node) {
    for (auto &expression : node->expressions()) {
        visit(expression);
    }
}

void Visitor::visit_name(Name *node) {
    for (auto &parameter : node->parameters()) {
        visit(parameter);
    }
}

void Visitor::visit_selector(Selector *node) {
    if (node->operand()) {
        visit(node->operand());
    }

    visit(node->field());
}

void Visitor::visit_type_name(TypeName *node) {
    for (auto &parameter : node->parameters()) {
        visit(parameter);
    }
}

void Visitor::visit_decl_name(DeclName *node) {
    visit(node->selector());

    for (auto &parameter : node->parameters()) {
        visit(parameter);
    }
}

void Visitor::visit_variable_declaration(VariableDeclaration *node) {
    visit(node->name());

    if (node->given_type()) {
        visit(node->given_type());
    }
}

void Visitor::visit_int(Int *node) {

}

void Visitor::visit_float(Float *node) {

}

void Visitor::visit_complex(Complex *node) {

}

void Visitor::visit_string(String *node) {

}

void Visitor::visit_list(List *node) {
    for (auto &element : node->elements()) {
        visit(element);
    }
}

void Visitor::visit_tuple(Tuple *node) {
    for (auto &element : node->elements()) {
        visit(element);
    }
}

void Visitor::visit_dictionary(Dictionary *node) {
    auto &keys = node->keys();
    auto &values = node->values();

    for (size_t i = 0; i < keys.size(); i++) {
        visit(keys[i]);
        visit(values[i]);
    }

}

void Visitor::visit_call(Call *node) {
    visit(node->operand());

    for (auto &positional_argument : node->positional_arguments()) {
        visit(positional_argument);
    }

    for (auto &keyword_argument : node->keyword_arguments()) {
        visit(keyword_argument.second);
    }

}

void Visitor::visit_ccall(CCall *node) {
    for (auto &parameter : node->parameters()) {
        visit(parameter);
    }

    visit(node->return_type());

    for (auto &argument : node->arguments()) {
        visit(argument);
    }
}

void Visitor::visit_cast(Cast *node) {
    visit(node->operand());
    visit(node->new_type());
}

void Visitor::visit_assignment(Assignment *node) {
    visit(node->lhs());

    if (!node->builtin()) {
        visit(node->rhs());
    }
}

void Visitor::visit_while(While *node) {
    visit(node->condition());
    visit(node->body());
}

void Visitor::visit_if(If *node) {
    visit(node->condition());

    visit(node->true_case());

    if (node->false_case()) {
        visit(node->false_case());
    }
}

void Visitor::visit_return(Return *node) {
    visit(node->expression());
}

void Visitor::visit_spawn(Spawn *node) {
    visit(node->call());
}

void Visitor::visit_case(Case *node) {
    visit(node->condition());

    if (node->assignment()) {
        visit(node->assignment());
    }

    visit(node->body());
}

void Visitor::visit_switch(Switch *node) {
    visit(node->expression());

    for (auto &case_ : node->cases()) {
        visit(case_);
    }

    if (node->default_case()) {
        visit(node->default_case());
    }
}

void Visitor::visit_parameter(Parameter *node) {
    visit(node->name());
    visit(node->given_type());
}

void Visitor::visit_let(Let *node) {
    visit(node->assignment());

    if (node->body()) {
        visit(node->body());
    }
}

void Visitor::visit_def(Def *node) {
    for (auto &instance : node->instances()) {
        visit(instance);
    }
}

void Visitor::visit_def_instance(DefInstance *node) {
    visit(node->name());

    for (auto &parameter : node->parameters()) {
        visit(parameter);
    }

    if (node->builtin() || node->return_type()) {
        visit(node->return_type());
    }

    if (!node->builtin()) {
        visit(node->body());
    }
}

void Visitor::visit_type_decl(TypeDecl *node) {
    visit(node->name());

    if (node->alias()) {
        visit(node->alias());
    } else {
        auto &field_names = node->field_names();
        auto &field_types = node->field_types();

        for (size_t i = 0; i < field_names.size(); i++) {
            visit(field_names[i]);
            visit(field_types[i]);
        }
    }
}

void Visitor::visit_module(Module *node) {
    visit(node->name());
    visit(node->body());
}

void Visitor::visit_import(Import *node) {
    visit(node->path());
}

void Visitor::visit_source_file(SourceFile *node) {
    for (auto &import : node->imports()) {
        visit(import);
    }

    visit(node->code());
}
