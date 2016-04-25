//
// Created by Thomas Leese on 25/04/2016.
//

#include <iostream>

#include "Generics.h"

using namespace Generics;

Duplicator::Duplicator() : m_collecting(true) {

}

void Duplicator::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Duplicator::visit(AST::Identifier *expression) {

}

void Duplicator::visit(AST::Type *type) {

}

void Duplicator::visit(AST::BooleanLiteral *boolean) {

}

void Duplicator::visit(AST::IntegerLiteral *expression) {

}

void Duplicator::visit(AST::FloatLiteral *expression) {

}

void Duplicator::visit(AST::ImaginaryLiteral *imaginary) {

}

void Duplicator::visit(AST::StringLiteral *expression) {

}

void Duplicator::visit(AST::SequenceLiteral *sequence) {

}

void Duplicator::visit(AST::MappingLiteral *mapping) {

}

void Duplicator::visit(AST::Argument *argument) {

}

void Duplicator::visit(AST::Call *expression) {

}

void Duplicator::visit(AST::CCall *expression) {

}

void Duplicator::visit(AST::Assignment *expression) {

}

void Duplicator::visit(AST::Selector *expression) {

}

void Duplicator::visit(AST::Index *expression) {

}

void Duplicator::visit(AST::Comma *expression) {

}

void Duplicator::visit(AST::While *expression) {

}

void Duplicator::visit(AST::For *expression) {

}

void Duplicator::visit(AST::If *expression) {

}

void Duplicator::visit(AST::Return *expression) {

}

void Duplicator::visit(AST::Spawn *expression) {

}

void Duplicator::visit(AST::Parameter *parameter) {

}

void Duplicator::visit(AST::VariableDefinition *definition) {

}

void Duplicator::visit(AST::FunctionDefinition *definition) {
    if (m_collecting) {
        if (!definition->parameters.empty()) {
            m_generic_functions.push_back(definition);
        }
    } else {

    }
}

void Duplicator::visit(AST::TypeDefinition *definition) {
    if (m_collecting) {
        if (!definition->parameters.empty()) {
            m_generic_types.push_back(definition);
        }
    } else {

    }
}

void Duplicator::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Duplicator::visit(AST::ExpressionStatement *statement) {

}

void Duplicator::visit(AST::ImportStatement *statement) {

}

void Duplicator::visit(AST::SourceFile *module) {
    module->code->accept(this);

    m_collecting = false;

    std::cout << "Identified: " << m_generic_types.size() << " generic types." << std::endl;
    std::cout << "Identified: " << m_generic_functions.size() << " generic functions." << std::endl;

    module->code->accept(this);
}
