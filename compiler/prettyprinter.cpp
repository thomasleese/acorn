//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "ast.h"
#include "types.h"

#include "prettyprinter.h"

using namespace acorn;

PrettyPrinter::PrettyPrinter()
        : indent(0)
{

}

std::string type_of(ast::Expression *expression)
{
    return expression->type_name();
}

void PrettyPrinter::visit(ast::Block *codeBlock)
{
    ss << indentation() << "(Block [" << type_of(codeBlock) << "]\n";
    indent++;

    for (auto statement : codeBlock->expressions()) {
        statement->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Name *identifier)
{
    if (identifier->has_parameters()) {
        ss << indentation() << "(Name " << identifier->value() << " [" << type_of(identifier) << "]\n";
        indent++;

        for (auto parameter : identifier->parameters()) {
            parameter->accept(this);
        }

        indent--;
        ss << indentation() << ")\n";
    } else {
        ss << indentation() << "(Name " << identifier->value() << " [" << type_of(identifier) << "])\n";
    }
}

void PrettyPrinter::visit(ast::VariableDeclaration *node)
{
    ss << indentation() << "(VariableDeclaration [" << type_of(node) << "]\n";
    indent++;

    node->name()->accept(this);

    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Int *expression)
{
    ss << indentation() << "(Int " << expression->value() << " [" << type_of(expression) << "])\n";
}

void PrettyPrinter::visit(ast::Float *expression)
{
    ss << indentation() << "(Float " << expression->value() << " [" << type_of(expression) << "])\n";
}

void PrettyPrinter::visit(ast::Complex *imaginary)
{
    ss << indentation() << "(Complex " << imaginary->value << ")\n";
}

void PrettyPrinter::visit(ast::String *expression)
{
    ss << indentation() << "(String " << expression->value() << ")\n";
}

void PrettyPrinter::visit(ast::List *sequence)
{
    ss << indentation() << "(List\n";
    indent++;

    for (auto element : sequence->elements()) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Dictionary *dict)
{
    ss << indentation() << "(Dictionary\n";
    indent++;

    for (size_t i = 0; i < dict->no_elements(); i++) {
        dict->get_key(i)->accept(this);
        dict->get_value(i)->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Tuple *expression)
{
    ss << indentation() << "(Tuple\n";
    indent++;

    for (auto element : expression->elements()) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Call *expression)
{
    ss << indentation() << "(Call [" << type_of(expression) << "]\n";
    indent++;

    expression->operand->accept(this);

    for (auto argument : expression->positional_arguments()) {
        argument->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::CCall *ccall)
{
    ss << indentation() << "(CCall [" << type_of(ccall) << "]\n";
    indent++;

    ccall->name->accept(this);

    for (auto parameter : ccall->parameters) {
        parameter->accept(this);
    }

    ccall->given_return_type->accept(this);

    for (auto argument : ccall->arguments) {
        argument->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Cast *cast)
{
    ss << indentation() << "(Cast\n";
    indent++;

    cast->operand->accept(this);
    cast->new_type->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Assignment *expression)
{
    ss << indentation() << "(Assignment [" << type_of(expression) << "]\n";
    indent++;

    expression->lhs->accept(this);
    expression->rhs->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Selector *expression)
{
    ss << indentation() << "(Selector [" << type_of(expression) << "]\n";
    indent++;

    expression->operand->accept(this);
    expression->name->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::While *expression)
{
    ss << indentation() << "(While [" << type_of(expression) << "]\n";
    indent++;

    expression->condition()->accept(this);
    expression->body()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::If *expression)
{
    ss << indentation() << "(If [" << type_of(expression) << "]\n";
    indent++;

    expression->condition->accept(this);
    expression->true_case->accept(this);

    if (expression->false_case) {
        expression->false_case->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Return *expression)
{
    ss << indentation() << "(Return [" << type_of(expression) << "]\n";
    indent++;

    expression->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Spawn *expression)
{
    ss << indentation() << "(Spawn [" << type_of(expression) << "]\n";
    indent++;

    expression->call->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Switch *expression)
{
    ss << indentation() << "(Switch [" << type_of(expression) << "]\n";
    indent++;

    expression->expression()->accept(this);

    for (auto entry : expression->cases()) {
        entry->condition()->accept(this);

        if (entry->assignment()) {
            entry->assignment()->accept(this);
        }

        entry->body()->accept(this);
    }

    if (expression->default_case()) {
        expression->default_case()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Parameter *parameter)
{
    ss << indentation() << "(Parameter\n";
    indent++;

    parameter->name()->accept(this);
    parameter->given_type()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Let *definition)
{
    ss << indentation() << "(Let [" << type_of(definition) << "]\n";
    indent++;

    definition->assignment->accept(this);

    if (definition->has_body()) {
        definition->body()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Def *definition)
{
    ss << indentation() << "(Def [" << type_of(definition) << "]\n";
    indent++;

    definition->name()->accept(this);

    for (auto parameter : definition->parameters()) {
        parameter->accept(this);
    }

    if (definition->has_given_return_type()) {
        definition->given_return_type()->accept(this);
    }

    if (definition->builtin()) {
        ss << "<builtin>";
    } else {
        definition->body()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Type *definition)
{
    ss << indentation() << "(Type\n";
    indent++;

    definition->name()->accept(this);

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        for (size_t i = 0; i < definition->field_names.size(); i++) {
            definition->field_names[i]->accept(this);
            definition->field_types[i]->accept(this);
        }
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Module *module) {
    ss << indentation() << "(Module [" << type_of(module) << "]\n";

    indent++;
    module->name()->accept(this);
    module->body()->accept(this);
    indent--;

    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Import *statement)
{
    ss << indentation() << "(Import [" << type_of(statement) << "]\n";
    indent++;

    statement->path->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::SourceFile *module)
{
    ss << indentation() << "(SourceFile " << module->name << " [" << type_of(module) << "]\n";

    indent++;
    module->code->accept(this);
    indent--;

    ss << indentation() << ")\n";
}

std::string PrettyPrinter::indentation()
{
    std::string s;
    for (int i = 0; i < indent; i++) {
        s += " ";
    }
    return s;
}

std::string PrettyPrinter::str()
{
    return ss.str();
}

void PrettyPrinter::print()
{
    std::cout << str() << std::endl;
}
