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

std::string type_of(ast::Node *node) {
    return node->type_name();
}

void PrettyPrinter::visit_block(ast::Block *node) {
    ss << indentation() << "(Block [" << type_of(node) << "]\n";
    indent++;

    ast::Visitor::visit_block(node);

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

        for (auto &parameter : node->parameters()) {
            visit(parameter.get());
        }

        indent--;
        ss << indentation() << ")\n";
    }

}

void PrettyPrinter::visit_selector(ast::Selector *node) {
    ss << indentation() << "(Selector [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_selector(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_variable_declaration(ast::VariableDeclaration *node) {
    ss << indentation() << "(VariableDeclaration [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_variable_declaration(node);

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

    ast::Visitor::visit_list(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_tuple(ast::Tuple *node) {
    ss << indentation() << "(Tuple\n";
    indent++;

    Visitor::visit_tuple(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_dictionary(ast::Dictionary *node) {
    ss << indentation() << "(Dictionary\n";
    indent++;

    ast::Visitor::visit_dictionary(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_call(ast::Call *node) {
    ss << indentation() << "(Call [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_call(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_ccall(ast::CCall *node) {
    ss << indentation() << "(CCall [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_ccall(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_cast(ast::Cast *node) {
    ss << indentation() << "(Cast\n";
    indent++;

    Visitor::visit_cast(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_assignment(ast::Assignment *node) {
    ss << indentation() << "(Assignment [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_assignment(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_while(ast::While *node) {
    ss << indentation() << "(While [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_while(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_if(ast::If *node) {
    ss << indentation() << "(If [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_if(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_return(ast::Return *node) {
    ss << indentation() << "(Return [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_return(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_spawn(ast::Spawn *node) {
    ss << indentation() << "(Spawn [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_spawn(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_switch(ast::Switch *node) {
    ss << indentation() << "(Switch [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_switch(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_parameter(ast::Parameter *node) {
    ss << indentation() << "(Parameter\n";
    indent++;

    Visitor::visit_parameter(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_let(ast::Let *node) {
    ss << indentation() << "(Let [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_let(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_def(ast::Def *node) {
    ss << indentation() << "(Def [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_def(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_def_instance(ast::DefInstance *node) {
    ss << indentation() << "(DefInstance [" << type_of(node) << "]\n";
    indent++;

    if (node->builtin()) {
        ss << "<builtin>";
    }

    Visitor::visit_def_instance(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_type_decl(ast::TypeDecl *node) {
    ss << indentation() << "(Type\n";
    indent++;

    Visitor::visit_type_decl(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_module(ast::Module *node) {
    ss << indentation() << "(Module [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_module(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_import(ast::Import *node) {
    ss << indentation() << "(Import [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_import(node);

    indent--;
    ss << indentation() << ")\n";

}

void PrettyPrinter::visit_source_file(ast::SourceFile *node) {
    ss << indentation() << "(SourceFile " << node->name() << " [" << type_of(node) << "]\n";
    indent++;

    Visitor::visit_source_file(node);

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
