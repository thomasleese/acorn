//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>

#include "Lexer.h"
#include "Errors.h"
#include "Builtins.h"

#include "SymbolTable.h"

using namespace SymbolTable;

Namespace::Namespace(Namespace *parent) : m_parent(parent) {

}

Namespace::~Namespace() {

}

Symbol *Namespace::lookup(AST::Node *currentNode, std::string name) const {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        if (m_parent) {
            return m_parent->lookup(currentNode, name);
        } else {
            throw Errors::UndefinedError(currentNode, name);
        }
    }

    return it->second;
}

void Namespace::insert(AST::Node *currentNode, Symbol *symbol) {
    auto it = m_symbols.find(symbol->name);
    if (it != m_symbols.end()) {
        throw Errors::RedefinedError(currentNode, symbol->name);
    }

    m_symbols[symbol->name] = symbol;
}

Symbol::Symbol(std::string name) {
    this->name = name;
    this->type = nullptr;
    this->nameSpace = 0;
}

Builder::Builder() {
    m_root = m_current = new Namespace();

    Builtins::fill_symbol_table(m_root);
}

bool Builder::isAtRoot() const {
    return m_root == m_current;
}

Namespace *Builder::rootNamespace() {
    return m_root;
}

void Builder::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Builder::visit(AST::Identifier *expression) {

}

void Builder::visit(AST::BooleanLiteral *boolean) {

}

void Builder::visit(AST::IntegerLiteral *expression) {

}

void Builder::visit(AST::FloatLiteral *expression) {

}

void Builder::visit(AST::ImaginaryLiteral *imaginary) {

}

void Builder::visit(AST::StringLiteral *expression) {

}

void Builder::visit(AST::SequenceLiteral *sequence) {

}

void Builder::visit(AST::MappingLiteral *mapping) {

}

void Builder::visit(AST::Argument *argument) {

}

void Builder::visit(AST::Call *expression) {

}

void Builder::visit(AST::Assignment *expression) {

}

void Builder::visit(AST::Selector *expression) {

}

void Builder::visit(AST::While *expression) {

}

void Builder::visit(AST::For *expression) {

}

void Builder::visit(AST::If *expression) {

}

void Builder::visit(AST::Type *type) {

}

void Builder::visit(AST::Cast *cast) {

}

void Builder::visit(AST::Parameter *parameter) {
    Symbol *symbol = new Symbol(parameter->name->name);
    m_current->insert(parameter, symbol);
}

void Builder::visit(AST::VariableDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->name);
    m_current->insert(definition, symbol);
}

void Builder::visit(AST::FunctionDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->name);
    m_current->insert(definition, symbol);

    symbol->nameSpace = new Namespace(m_current);

    Namespace *oldNamespace = m_current;
    m_current = symbol->nameSpace;

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->code->accept(this);

    m_current = oldNamespace;
}

void Builder::visit(AST::TypeDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->name->name);
    m_current->insert(definition, symbol);

    symbol->nameSpace = new Namespace(m_current);

    Namespace *oldNamespace = m_current;
    m_current = symbol->nameSpace;

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->name->name);
        sym->type = new Types::Parameter(parameter->name->name);
        m_current->insert(definition, sym);
    }

    m_current = oldNamespace;
}

void Builder::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Builder::visit(AST::ExpressionStatement *statement) {

}

void Builder::visit(AST::Module *module) {
    Symbol *symbol = new Symbol(module->name);
    m_current->insert(module, symbol);

    symbol->nameSpace = new Namespace(m_current);

    Namespace *oldNamespace = m_current;
    m_current = symbol->nameSpace;

    module->code->accept(this);

    m_current = oldNamespace;
}
