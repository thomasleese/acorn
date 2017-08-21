//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "acorn/ast/nodes.h"
#include "acorn/typesystem/types.h"

#include "acorn/prettyprinter.h"

using namespace acorn;

PrettyPrinter::PrettyPrinter() : indent(0) {

}

std::string type_of(ast::Expression *node) {
    return node->type_name();
}

void PrettyPrinter::visit_block(ast::Block *node) {
    ss << indentation() << "(Block [" << type_of(node) << "]\n";
    indent++;

    accept_many(node->expressions());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_name(ast::Name *node) {
    auto &parameters = node->parameters();

    if (parameters.empty()) {
        ss << indentation() << "(Name " << node->value() << " [" << type_of(node) << "])\n";
    } else {
        ss << indentation() << "(Name " << node->value() << " [" << type_of(node) << "]\n";
        indent++;

        accept_many(parameters);

        indent--;
        ss << indentation() << ")\n";
    }
}

void PrettyPrinter::visit_variable_declaration(ast::VariableDeclaration *node) {
    ss << indentation() << "(VariableDeclaration [" << type_of(node) << "]\n";
    indent++;

    accept(node->name());
    accept_if_present(node->given_type());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_int(ast::Int *node) {
    ss << indentation() << "(Int " << node->value() << " [" << type_of(node) << "])\n";
}

void PrettyPrinter::visit_float(ast::Float *node) {
    ss << indentation() << "(Float " << node->value() << " [" << type_of(node) << "])\n";
}

void PrettyPrinter::visit_complex(ast::Complex *node) {
    ss << indentation() << "(Complex " << "<unknown>" << ")\n";
}

void PrettyPrinter::visit_string(ast::String *node) {
    ss << indentation() << "(String " << node->value() << ")\n";
}

void PrettyPrinter::visit_list(ast::List *node) {
    ss << indentation() << "(List\n";
    indent++;

    accept_many(node->elements());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_dictionary(ast::Dictionary *node) {
    ss << indentation() << "(Dictionary\n";
    indent++;

    for (size_t i = 0; i < node->elements_size(); i++) {
        node->key(i)->accept(this);
        node->value(i)->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_tuple(ast::Tuple *node) {
    ss << indentation() << "(Tuple\n";
    indent++;

    accept_many(node->elements());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_call(ast::Call *node) {
    ss << indentation() << "(Call [" << type_of(node) << "]\n";
    indent++;

    accept(node->operand());
    accept_many(node->positional_arguments());

    for (auto &entry : node->keyword_arguments()) {
        accept(entry.second);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_ccall(ast::CCall *node) {
    ss << indentation() << "(CCall [" << type_of(node) << "]\n";
    indent++;

    accept(node->name());
    accept_many(node->parameters());
    accept(node->given_return_type());
    accept_many(node->arguments());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_cast(ast::Cast *node) {
    ss << indentation() << "(Cast\n";
    indent++;

    accept(node->operand());
    accept(node->new_type());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_assignment(ast::Assignment *node) {
    ss << indentation() << "(Assignment [" << type_of(node) << "]\n";
    indent++;

    accept(node->lhs());

    if (!node->builtin()) {
        accept(node->rhs());
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_selector(ast::Selector *node) {
    ss << indentation() << "(Selector [" << type_of(node) << "]\n";
    indent++;

    accept(node->operand());
    accept(node->field());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_while(ast::While *node) {
    ss << indentation() << "(While [" << type_of(node) << "]\n";
    indent++;

    node->condition()->accept(this);
    node->body()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_if(ast::If *node) {
    ss << indentation() << "(If [" << type_of(node) << "]\n";
    indent++;

    node->condition()->accept(this);
    node->true_case()->accept(this);

    accept_if_present(node->false_case());

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_return(ast::Return *node) {
    ss << indentation() << "(Return [" << type_of(node) << "]\n";
    indent++;

    node->expression()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_spawn(ast::Spawn *node) {
    ss << indentation() << "(Spawn [" << type_of(node) << "]\n";
    indent++;

    node->call()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_switch(ast::Switch *node) {
    ss << indentation() << "(Switch [" << type_of(node) << "]\n";
    indent++;

    node->expression()->accept(this);

    for (auto &entry : node->cases()) {
        entry->condition()->accept(this);

        if (entry->has_assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);
    }

    if (node->has_default_case()) {
        node->default_case()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_parameter(ast::Parameter *node) {
    ss << indentation() << "(Parameter\n";
    indent++;

    node->name()->accept(this);
    node->given_type()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_let(ast::Let *node) {
    ss << indentation() << "(Let [" << type_of(node) << "]\n";
    indent++;

    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_def(ast::Def *node) {
    ss << indentation() << "(Def [" << type_of(node) << "]\n";
    indent++;

    accept(node->name());
    accept_many(node->parameters());
    accept_if_present(node->given_return_type());

    if (node->builtin()) {
        ss << "<builtin>";
    } else {
        accept(node->body());
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_type(ast::Type *node) {
    ss << indentation() << "(Type\n";
    indent++;

    accept(node->name());

    if (node->has_alias()) {
        node->alias()->accept(this);
    } else {
        for (size_t i = 0; i < node->field_names().size(); i++) {
            node->field_names()[i]->accept(this);
            node->field_types()[i]->accept(this);
        }
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_module(ast::Module *node) {
    ss << indentation() << "(Module [" << type_of(node) << "]\n";
    indent++;

    node->name()->accept(this);
    node->body()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_import(ast::Import *node) {
    ss << indentation() << "(Import [" << type_of(node) << "]\n";
    indent++;

    node->path()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit_source_file(ast::SourceFile *node) {
    ss << indentation() << "(SourceFile " << node->name() << " [" << type_of(node) << "]\n";
    indent++;

    for (auto &import : node->imports()) {
        import->accept(this);
    }

    node->code()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

std::string PrettyPrinter::indentation() {
    std::string s;
    for (int i = 0; i < indent; i++) {
        s += " ";
    }
    return s;
}

std::string PrettyPrinter::str() {
    return ss.str();
}

void PrettyPrinter::print() {
    std::cout << str() << std::endl;
}
