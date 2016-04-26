//
// Created by Thomas Leese on 25/04/2016.
//

#include <iostream>

#include "SymbolTable.h"
#include "Types.h"

#include "Preprocessor.h"

using namespace jet::preprocessor;

GenericsPass::GenericsPass(SymbolTable::Namespace *root_namespace) :
        m_collecting(true) {
    m_scope.push_back(root_namespace);
}

void GenericsPass::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void GenericsPass::visit(AST::Identifier *identifier) {

}

void GenericsPass::visit(AST::BooleanLiteral *boolean) {

}

void GenericsPass::visit(AST::IntegerLiteral *expression) {

}

void GenericsPass::visit(AST::FloatLiteral *expression) {

}

void GenericsPass::visit(AST::ImaginaryLiteral *imaginary) {

}

void GenericsPass::visit(AST::StringLiteral *expression) {

}

void GenericsPass::visit(AST::SequenceLiteral *sequence) {

}

void GenericsPass::visit(AST::MappingLiteral *mapping) {

}

void GenericsPass::visit(AST::Argument *argument) {

}

void GenericsPass::visit(AST::Call *expression) {
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

void GenericsPass::visit(AST::CCall *expression) {

}

void GenericsPass::visit(AST::Assignment *expression) {

}

void GenericsPass::visit(AST::Selector *expression) {

}

void GenericsPass::visit(AST::Index *expression) {

}

void GenericsPass::visit(AST::Comma *expression) {

}

void GenericsPass::visit(AST::While *expression) {

}

void GenericsPass::visit(AST::For *expression) {

}

void GenericsPass::visit(AST::If *expression) {

}

void GenericsPass::visit(AST::Return *expression) {

}

void GenericsPass::visit(AST::Spawn *expression) {

}

void GenericsPass::visit(AST::Parameter *parameter) {

}

void GenericsPass::visit(AST::VariableDefinition *definition) {
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

void GenericsPass::visit(AST::FunctionDefinition *definition) {
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

void GenericsPass::visit(AST::TypeDefinition *definition) {
    if (m_collecting) {
        if (!definition->parameters.empty()) {
            m_types[definition] = std::vector<std::vector<AST::Identifier *> >();
        }
    } else {

    }
}

void GenericsPass::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void GenericsPass::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void GenericsPass::visit(AST::ImportStatement *statement) {

}

void GenericsPass::visit(AST::SourceFile *module) {
    module->code->accept(this);

    m_collecting = false;

    std::cout << "Identified: " << m_types.size() << " generic types." << std::endl;
    std::cout << "Identified: " << m_functions.size() << " generic functions." << std::endl;

    module->code->accept(this);
}
