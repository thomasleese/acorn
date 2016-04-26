//
// Created by Thomas Leese on 25/04/2016.
//

#include <iostream>

#include "SymbolTable.h"
#include "Types.h"

#include "Generics.h"

using namespace Generics;

Duplicator::Duplicator(SymbolTable::Namespace *root_namespace) :
        m_collecting(true) {
    m_scope.push_back(root_namespace);
}

void Duplicator::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Duplicator::visit(AST::Identifier *identifier) {

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
    if (m_collecting) {
        auto identifier = dynamic_cast<AST::Identifier *>(expression->operand);
        assert(identifier);

        if (identifier->has_parameters()) {
            std::cout << "Identified a use!" << std::endl;

            SymbolTable::Symbol *symbol = m_scope.back()->lookup(identifier);
            assert(dynamic_cast<Types::Function *>(symbol->type));

            // for each method, we have to create a
        }
    }
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
    if (m_collecting) {
        if (definition->typeNode) {
            if (!definition->typeNode->parameters.empty()) {
                // has parameters

                std::cout << "Identified a use!" << std::endl;
            }
        }

        definition->expression->accept(this);
    }
}

void Duplicator::visit(AST::FunctionDefinition *definition) {
    SymbolTable::Symbol *functionSymbol = m_scope.back()->lookup(definition, definition->name->value);
    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup_by_node(definition);
    m_scope.push_back(symbol->nameSpace);

    if (m_collecting) {
        if (!definition->parameters.empty()) {
            m_functions[definition] = std::vector<std::vector<AST::Identifier *> >();
        }
    } else {
        definition->code->accept(this);
    }

    m_scope.pop_back();
}

void Duplicator::visit(AST::TypeDefinition *definition) {
    if (m_collecting) {
        if (!definition->parameters.empty()) {
            m_types[definition] = std::vector<std::vector<AST::Identifier *> >();
        }
    } else {

    }
}

void Duplicator::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Duplicator::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Duplicator::visit(AST::ImportStatement *statement) {

}

void Duplicator::visit(AST::SourceFile *module) {
    module->code->accept(this);

    m_collecting = false;

    std::cout << "Identified: " << m_types.size() << " generic types." << std::endl;
    std::cout << "Identified: " << m_functions.size() << " generic functions." << std::endl;

    module->code->accept(this);
}
