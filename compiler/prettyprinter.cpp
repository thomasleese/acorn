//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "ast/nodes.h"
#include "types.h"

#include "prettyprinter.h"

using namespace acorn;

PrettyPrinter::PrettyPrinter() : indent(0) {

}

std::string type_of(ast::Expression *node) {
    return node->type_name();
}

void PrettyPrinter::visit(ast::Block *node) {
    ss << indentation() << "(Block [" << type_of(node) << "]\n";
    indent++;

    for (auto &expression : node->expressions()) {
        expression->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Name *node) {
    if (node->has_parameters()) {
        ss << indentation() << "(Name " << node->value() << " [" << type_of(node) << "]\n";
        indent++;

        for (auto parameter : node->parameters()) {
            parameter->accept(this);
        }

        indent--;
        ss << indentation() << ")\n";
    } else {
        ss << indentation() << "(Name " << node->value() << " [" << type_of(node) << "])\n";
    }
}

void PrettyPrinter::visit(ast::VariableDeclaration *node) {
    ss << indentation() << "(VariableDeclaration [" << type_of(node) << "]\n";
    indent++;

    node->name()->accept(this);

    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Int *node) {
    ss << indentation() << "(Int " << node->value() << " [" << type_of(node) << "])\n";
}

void PrettyPrinter::visit(ast::Float *node) {
    ss << indentation() << "(Float " << node->value() << " [" << type_of(node) << "])\n";
}

void PrettyPrinter::visit(ast::Complex *node) {
    ss << indentation() << "(Complex " << "<unknown>" << ")\n";
}

void PrettyPrinter::visit(ast::String *node) {
    ss << indentation() << "(String " << node->value() << ")\n";
}

void PrettyPrinter::visit(ast::List *node) {
    ss << indentation() << "(List\n";
    indent++;

    for (auto element : node->elements()) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Dictionary *node) {
    ss << indentation() << "(Dictionary\n";
    indent++;

    for (size_t i = 0; i < node->elements_size(); i++) {
        node->key(i)->accept(this);
        node->value(i)->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Tuple *node) {
    ss << indentation() << "(Tuple\n";
    indent++;

    for (auto element : node->elements()) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Call *node) {
    ss << indentation() << "(Call [" << type_of(node) << "]\n";
    indent++;

    node->operand()->accept(this);

    for (auto &argument : node->positional_arguments()) {
        argument->accept(this);
    }

    for (auto &entry : node->keyword_arguments()) {
        entry.second->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::CCall *node) {
    ss << indentation() << "(CCall [" << type_of(node) << "]\n";
    indent++;

    node->name()->accept(this);

    for (auto parameter : node->parameters()) {
        parameter->accept(this);
    }

    node->given_return_type()->accept(this);

    for (auto argument : node->arguments()) {
        argument->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Cast *node) {
    ss << indentation() << "(Cast\n";
    indent++;

    node->operand()->accept(this);
    node->new_type()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Assignment *node) {
    ss << indentation() << "(Assignment [" << type_of(node) << "]\n";
    indent++;

    node->lhs()->accept(this);

    if (!node->builtin()) {
        node->rhs()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Selector *node) {
    ss << indentation() << "(Selector [" << type_of(node) << "]\n";
    indent++;

    node->operand()->accept(this);
    node->field()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::While *node) {
    ss << indentation() << "(While [" << type_of(node) << "]\n";
    indent++;

    node->condition()->accept(this);
    node->body()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::If *node) {
    ss << indentation() << "(If [" << type_of(node) << "]\n";
    indent++;

    node->condition()->accept(this);
    node->true_case()->accept(this);

    if (node->has_false_case()) {
        node->false_case()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Return *node) {
    ss << indentation() << "(Return [" << type_of(node) << "]\n";
    indent++;

    node->expression()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Spawn *node) {
    ss << indentation() << "(Spawn [" << type_of(node) << "]\n";
    indent++;

    node->call()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Switch *node) {
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

void PrettyPrinter::visit(ast::Parameter *node) {
    ss << indentation() << "(Parameter\n";
    indent++;

    node->name()->accept(this);
    node->given_type()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Let *node) {
    ss << indentation() << "(Let [" << type_of(node) << "]\n";
    indent++;

    node->assignment()->accept(this);

    if (node->has_body()) {
        node->body()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Def *node) {
    ss << indentation() << "(Def [" << type_of(node) << "]\n";
    indent++;

    node->name()->accept(this);

    for (auto parameter : node->parameters()) {
        parameter->accept(this);
    }

    if (node->has_given_return_type()) {
        node->given_return_type()->accept(this);
    }

    if (node->builtin()) {
        ss << "<builtin>";
    } else {
        node->body()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Type *node)
{
    ss << indentation() << "(Type\n";
    indent++;

    node->name()->accept(this);

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

void PrettyPrinter::visit(ast::Module *node) {
    ss << indentation() << "(Module [" << type_of(node) << "]\n";
    indent++;

    node->name()->accept(this);
    node->body()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Import *node) {
    ss << indentation() << "(Import [" << type_of(node) << "]\n";
    indent++;

    node->path()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::SourceFile *node) {
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
