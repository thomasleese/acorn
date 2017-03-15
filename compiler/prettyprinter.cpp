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

std::string type_of(ast::Node *node)
{
    if (node->type) {
        return node->type->name();
    } else {
        return "null";
    }
}

void PrettyPrinter::visit(ast::Block *codeBlock)
{
    ss << indentation() << "(Block [" << type_of(codeBlock) << "]\n";
    indent++;

    for (auto statement : codeBlock->expressions) {
        statement->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Identifier *identifier)
{
    if (identifier->has_parameters()) {
        ss << indentation() << "(Identifier " << identifier->value << " [" << type_of(identifier) << "]\n";
        indent++;

        for (auto parameter : identifier->parameters) {
            parameter->accept(this);
        }

        indent--;
        ss << indentation() << ")\n";
    } else {
        ss << indentation() << "(Identifier " << identifier->value << " [" << type_of(identifier) << "])\n";
    }
}

void PrettyPrinter::visit(ast::VariableDeclaration *node)
{
    ss << indentation() << "(VariableDeclaration\n";
    indent++;

    node->name()->accept(this);

    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::IntegerLiteral *expression)
{
    ss << indentation() << "(IntegerLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(ast::FloatLiteral *expression)
{
    ss << indentation() << "(FloatLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(ast::ImaginaryLiteral *imaginary)
{
    ss << indentation() << "(ImaginaryLiteral " << imaginary->value << ")\n";
}

void PrettyPrinter::visit(ast::StringLiteral *expression)
{
    ss << indentation() << "(StringLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(ast::SequenceLiteral *sequence)
{
    ss << indentation() << "(SequenceLiteral\n";
    indent++;

    for (auto element : sequence->elements) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::MappingLiteral *mapping)
{
    ss << indentation() << "(MappingLiteral\n";
    indent++;

    for (unsigned long i = 0; i < mapping->keys.size(); i++) {
        mapping->keys[i]->accept(this);
        mapping->values[i]->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::RecordLiteral *expression)
{
    ss << indentation() << "(RecordLiteral\n";
    indent++;

    expression->name->accept(this);

    for (unsigned long i = 0; i < expression->field_names.size(); i++) {
        expression->field_names[i]->accept(this);
        expression->field_values[i]->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::TupleLiteral *expression)
{
    ss << indentation() << "(TupleLiteral\n";
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

    for (auto argument : expression->arguments) {
        argument->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::CCall *ccall)
{
    ss << indentation() << "(CCall\n";
    indent++;

    ccall->name->accept(this);

    for (auto parameter : ccall->parameters) {
        parameter->accept(this);
    }

    ccall->returnType->accept(this);

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
    ss << indentation() << "(Assignment\n";
    indent++;

    expression->lhs->accept(this);
    expression->rhs->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Selector *expression)
{
    ss << indentation() << "(Selector\n";
    indent++;

    expression->operand->accept(this);
    expression->name->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::While *expression)
{
    ss << indentation() << "(While\n";
    indent++;

    expression->condition()->accept(this);
    expression->code()->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::If *expression)
{
    ss << indentation() << "(If\n";
    indent++;

    expression->condition->accept(this);
    expression->trueCode->accept(this);

    if (expression->falseCode) {
        expression->falseCode->accept(this);
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
    ss << indentation() << "(Spawn\n";
    indent++;

    expression->call->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Switch *expression)
{
    ss << indentation() << "(Switch\n";
    indent++;

    expression->expression()->accept(this);

    for (auto entry : expression->cases()) {
        entry->condition()->accept(this);

        if (entry->assignment()) {
            entry->assignment()->accept(this);
        }

        entry->code()->accept(this);
    }

    if (expression->default_block()) {
        expression->default_block()->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Parameter *parameter)
{
    ss << indentation() << "(Parameter\n";
    indent++;

    parameter->name->accept(this);
    parameter->typeNode->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::VariableDefinition *definition)
{
    ss << indentation() << "(VariableDefinition\n";
    indent++;

    definition->assignment->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::FunctionDefinition *definition)
{
    ss << indentation() << "(FunctionDefinition\n";
    indent++;

    definition->name->accept(this);

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->returnType->accept(this);
    definition->code->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::TypeDefinition *definition)
{
    ss << indentation() << "(TypeDefinition\n";
    indent++;

    definition->name->accept(this);

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

void PrettyPrinter::visit(ast::DefinitionExpression *statement)
{
    ss << indentation() << "(DefinitionExpression\n";
    indent++;

    statement->definition->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::ImportExpression *statement)
{
    ss << indentation() << "(ImportExpression\n";
    indent++;

    statement->path->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::SourceFile *module)
{
    ss << indentation() << "(SourceFile " << module->name << "\n";

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
