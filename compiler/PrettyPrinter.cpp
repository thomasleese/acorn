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

void PrettyPrinter::visit(AST::Identifier *identifier) {
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

    if (argument->name) {
        argument->name->accept(this);
    } else {
        ss << indentation() << "[Unnammed]\n";
    }

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

void PrettyPrinter::visit(AST::CCall *ccall) {
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

void PrettyPrinter::visit(AST::Cast *cast) {
    ss << indentation() << "(Cast\n";
    indent++;

    cast->operand->accept(this);
    cast->new_type->accept(this);

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

    expression->operand->accept(this);
    expression->name->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Index *expression) {
    ss << indentation() << "(Index\n";
    indent++;

    expression->operand->accept(this);
    expression->index->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Comma *expression) {
    ss << indentation() << "(Comma\n";
    indent++;

    expression->lhs->accept(this);
    expression->rhs->accept(this);

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

void PrettyPrinter::visit(AST::Return *expression) {
    ss << indentation() << "(Return\n";
    indent++;

    expression->expression->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Spawn *expression) {
    ss << indentation() << "(Spawn\n";
    indent++;

    expression->call->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Sizeof *expression) {
    ss << indentation() << "(Sizeof\n";
    indent++;

    expression->identifier->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Strideof *expression) {
    ss << indentation() << "(Strideof\n";
    indent++;

    expression->identifier->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::Parameter *parameter) {
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

void PrettyPrinter::visit(AST::VariableDefinition *definition) {
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

void PrettyPrinter::visit(AST::FunctionDefinition *definition) {
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

void PrettyPrinter::visit(AST::TypeDefinition *definition) {
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

void PrettyPrinter::visit(AST::ImportStatement *statement) {
    ss << indentation() << "(ImportStatement\n";
    indent++;

    statement->path->accept(this);

    indent--;
    ss << indentation() << ")\n";
}

void PrettyPrinter::visit(AST::SourceFile *module) {
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
