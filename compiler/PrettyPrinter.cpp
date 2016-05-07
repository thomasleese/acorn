//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "ast/nodes.h"

#include "PrettyPrinter.h"

using namespace jet;

PrettyPrinter::PrettyPrinter() {
    indent = 0;
}

void PrettyPrinter::visit(ast::CodeBlock *codeBlock) {
    ss << indentation() << "(CodeBlock\n";
    indent++;

    for (auto statement : codeBlock->statements) {
        statement->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Identifier *identifier) {
    if (identifier->has_parameters()) {
        ss << indentation() << "(Identifier " << identifier->value << "\n";
        indent++;

        for (auto parameter : identifier->parameters) {
            parameter->accept(this);
        }

        indent--;
        ss << indentation() << ")\n";
    } else {
        ss << indentation() << "(Identifier " << identifier->value << ")\n";
    }
}

void PrettyPrinter::visit(ast::BooleanLiteral *boolean) {
    ss << indentation() << "(BooleanLiteral " << boolean->value << ")\n";
}

void PrettyPrinter::visit(ast::IntegerLiteral *expression) {
    ss << indentation() << "(IntegerLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(ast::FloatLiteral *expression) {
    ss << indentation() << "(FloatLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(ast::ImaginaryLiteral *imaginary) {
    ss << indentation() << "(ImaginaryLiteral " << imaginary->value << ")\n";
}

void PrettyPrinter::visit(ast::StringLiteral *expression) {
    ss << indentation() << "(StringLiteral " << expression->value << ")\n";
}

void PrettyPrinter::visit(ast::SequenceLiteral *sequence) {
    ss << indentation() << "(SequenceLiteral\n";
    indent++;

    for (auto element : sequence->elements) {
        element->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::MappingLiteral *mapping) {
    ss << indentation() << "(MappingLiteral\n";
    indent++;

    for (unsigned long i = 0; i < mapping->keys.size(); i++) {
        mapping->keys[i]->accept(this);
        mapping->values[i]->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::RecordLiteral *expression) {
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

void PrettyPrinter::visit(ast::Argument *argument) {
    ss << indentation() << "(Argument\n";
    indent++;

    if (argument->name) {
        argument->name->accept(this);
    } else {
        ss << indentation() << "[Unnammed]\n";
    }

    argument->value->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Call *expression) {
    ss << indentation() << "(Call\n";
    indent++;

    expression->operand->accept(this);

    for (auto argument : expression->arguments) {
        argument->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::CCall *ccall) {
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

void PrettyPrinter::visit(ast::Cast *cast) {
    ss << indentation() << "(Cast\n";
    indent++;

    cast->operand->accept(this);
    cast->new_type->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Assignment *expression) {
    ss << indentation() << "(Assignment\n";
    indent++;

    expression->lhs->accept(this);
    expression->rhs->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Selector *expression) {
    ss << indentation() << "(Selector\n";
    indent++;

    expression->operand->accept(this);
    expression->name->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Index *expression) {
    ss << indentation() << "(Index\n";
    indent++;

    expression->operand->accept(this);
    expression->index->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Comma *expression) {
    ss << indentation() << "(Comma\n";
    indent++;

    expression->lhs->accept(this);
    expression->rhs->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::While *expression) {
    ss << indentation() << "(While\n";
    indent++;

    expression->condition->accept(this);
    expression->code->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::For *expression) {
    ss << indentation() << "(For\n";
    indent++;

    expression->name->accept(this);
    expression->iterator->accept(this);
    expression->code->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::If *expression) {
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

void PrettyPrinter::visit(ast::Return *expression) {
    ss << indentation() << "(Return\n";
    indent++;

    expression->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Spawn *expression) {
    ss << indentation() << "(Spawn\n";
    indent++;

    expression->call->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Sizeof *expression) {
    ss << indentation() << "(Sizeof\n";
    indent++;

    expression->identifier->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Strideof *expression) {
    ss << indentation() << "(Strideof\n";
    indent++;

    expression->identifier->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::Parameter *parameter) {
    ss << indentation() << "(Parameter\n";
    indent++;

    parameter->name->accept(this);
    parameter->typeNode->accept(this);

    if (parameter->defaultExpression) {
        parameter->defaultExpression->accept(this);
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::VariableDefinition *definition) {
    ss << indentation() << "(VariableDefinition\n";
    indent++;

    definition->name->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
    } else {
        ss << indentation() << "(null)\n";
    }
    definition->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::FunctionDefinition *definition) {
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

void PrettyPrinter::visit(ast::TypeDefinition *definition) {
    ss << indentation() << "(TypeDefinition\n";
    indent++;

    definition->name->accept(this);

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        for (int i = 0; i < definition->field_names.size(); i++) {
            definition->field_names[i]->accept(this);
            definition->field_types[i]->accept(this);
        }
    }

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::DefinitionStatement *statement) {
    ss << indentation() << "(DefinitionStatement\n";
    indent++;

    statement->definition->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::ExpressionStatement *statement) {
    ss << indentation() << "(ExpressionStatement\n";
    indent++;

    statement->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::ImportStatement *statement) {
    ss << indentation() << "(ImportStatement\n";
    indent++;

    statement->path->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(ast::SourceFile *module) {
    ss << indentation() << "(SourceFile " << module->name << "\n";

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
