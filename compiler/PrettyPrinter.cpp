//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "PrettyPrinter.h"

PrettyPrinter::PrettyPrinter() {
    indent = 0;
}

void PrettyPrinter::visit(AST::CodeBlock *codeBlock) {
    ss << indentation() << "(CodeBlock\n";
    indent++;

    for (auto statement : codeBlock->statements) {
        statement->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Identifier *expression) {
    ss << indentation() << "(Identifier " << expression->name << ")\n";
}

void PrettyPrinter::visit(AST::BooleanLiteral *boolean) {
    ss << indentation() << "(BooleanLiteral " << boolean->value << ")\n";
}

void PrettyPrinter::visit(AST::IntegerLiteral *expression) {
    ss << indentation() << "(IntegerLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(AST::FloatLiteral *expression) {
    ss << indentation() << "(FloatLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(AST::ImaginaryLiteral *imaginary) {
    ss << indentation() << "(ImaginaryLiteral " << imaginary->value << ")\n";
}

void PrettyPrinter::visit(AST::StringLiteral *expression) {
    ss << indentation() << "(StringLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(AST::SequenceLiteral *sequence) {
    ss << indentation() << "(SequenceLiteral\n";
    indent++;

    for (auto element : sequence->elements) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::MappingLiteral *mapping) {
    ss << indentation() << "(MappingLiteral\n";
    indent++;

    for (unsigned long i = 0; i < mapping->keys.size(); i++) {
        mapping->keys[i]->accept(this);
        mapping->values[i]->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Argument *argument) {
    ss << indentation() << "(Argument\n";
    indent++;

    argument->name->accept(this);
    argument->value->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Call *expression) {
    ss << indentation() << "(Call\n";
    indent++;

    expression->operand->accept(this);

    for (auto argument : expression->arguments) {
        argument->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Assignment *expression) {
    ss << indentation() << "(Assignment\n";
    indent++;

    expression->lhs->accept(this);
    expression->rhs->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Selector *expression) {
    ss << indentation() << "(Selector\n";
    indent++;

    expression->name->accept(this);
    expression->operand->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::While *expression) {
    ss << indentation() << "(While\n";
    indent++;

    expression->condition->accept(this);
    expression->code->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::For *expression) {
    ss << indentation() << "(For\n";
    indent++;

    expression->name->accept(this);
    expression->iterator->accept(this);
    expression->code->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::If *expression) {
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

void PrettyPrinter::visit(AST::Type *type) {
    ss << indentation() << "(Type\n";
    indent++;

    type->name->accept(this);

    for (auto parameter : type->parameters) {
        parameter->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Cast *cast) {
    ss << indentation() << "(Cast\n";
    indent++;

    cast->type->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Parameter *parameter) {
    ss << indentation() << "(Parameter\n";
    indent++;

    parameter->name->accept(this);
    parameter->cast->accept(this);

    if (parameter->defaultExpression) {
        parameter->defaultExpression->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::VariableDefinition *definition) {
    ss << indentation() << "(VariableDefinition\n";
    indent++;

    definition->name->accept(this);
    definition->cast->accept(this);
    definition->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::FunctionDefinition *definition) {
    ss << indentation() << "(FunctionDefinition\n";
    indent++;

    definition->name->accept(this);

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->returnCast->accept(this);
    definition->code->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::TypeDefinition *definition) {
    ss << indentation() << "(TypeDefinition\n";
    indent++;

    definition->name->accept(this);

    for (auto field : definition->fields) {
        field->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::DefinitionStatement *statement) {
    ss << indentation() << "(DefinitionStatement\n";
    indent++;

    statement->definition->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::ExpressionStatement *statement) {
    ss << indentation() << "(ExpressionStatement\n";
    indent++;

    statement->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Module *module) {
    ss << indentation() << "(Module " << module->name << "\n";

    indent++;
    module->code->accept(this);
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
