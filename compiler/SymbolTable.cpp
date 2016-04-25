//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "Lexer.h"
#include "Errors.h"
#include "Builtins.h"
#include "Types.h"

#include "SymbolTable.h"

using namespace SymbolTable;

Namespace::Namespace(Namespace *parent) : m_parent(parent) {

}

Namespace::~Namespace() {

}

bool Namespace::has(std::string name, bool follow_parents) const {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        if (follow_parents && m_parent) {
            return m_parent->has(name);
        } else {
            return false;
        }
    } else {
        return true;
    }
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

Symbol *Namespace::lookup(AST::Identifier *identifier) const {
    return lookup(identifier, identifier->name);
}

void Namespace::insert(AST::Node *currentNode, Symbol *symbol) {
    auto it = m_symbols.find(symbol->name);
    if (it != m_symbols.end()) {
        throw Errors::RedefinedError(currentNode, symbol->name);
    }

    m_symbols[symbol->name] = symbol;
}

void Namespace::rename(Symbol *symbol, std::string new_name) {
    auto it = m_symbols.find(symbol->name);
    assert(it != m_symbols.end());

    m_symbols.erase(it);
    symbol->name = new_name;
    insert(nullptr, symbol);
}

unsigned long Namespace::size() const {
    return m_symbols.size();
}

bool Namespace::is_root() const {
    return m_parent == nullptr;
}

std::string Namespace::to_string() const {
    std::stringstream ss;

    ss << "{";

    for (auto it = m_symbols.begin(); it != m_symbols.end(); it++) {
        auto symbol = it->second;
        ss << symbol->to_string() << ", ";
    }

    ss << "}";

    return ss.str();
}

Symbol::Symbol(std::string name) {
    this->name = name;
    this->type = nullptr;
    this->value = nullptr;
    this->nameSpace = nullptr;
}

std::string Symbol::to_string() const {
    std::stringstream ss;
    ss << this->name;

    if (this->type) {
        ss << ": " << this->type->name();
    }

    if (this->nameSpace) {
        ss << " " << this->nameSpace->to_string();
    }

    return ss.str();
}

Builder::Builder() {
    m_root = m_current = new Namespace(nullptr);

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

void Builder::visit(AST::Type *type) {

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

void Builder::visit(AST::CCall *expression) {

}

void Builder::visit(AST::Assignment *expression) {

}

void Builder::visit(AST::Selector *expression) {

}

void Builder::visit(AST::Index *expression) {

}

void Builder::visit(AST::Comma *expression) {

}

void Builder::visit(AST::While *expression) {

}

void Builder::visit(AST::For *expression) {

}

void Builder::visit(AST::If *expression) {

}

void Builder::visit(AST::Return *expression) {

}

void Builder::visit(AST::Spawn *expression) {

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
    Symbol *functionSymbol;
    if (m_current->has(definition->name->name, false)) {
        // we don't want to look in any parent scope when we're
        // defining a new function; it should follow the notion of
        // variables, i.e. we are hiding the previous binding
        functionSymbol = m_current->lookup(definition->name);
    } else {
        functionSymbol = new Symbol(definition->name->name);
        functionSymbol->type = new Types::Function();
        functionSymbol->nameSpace = new Namespace(m_current);
        m_current->insert(definition, functionSymbol);
    }

    // this is really hacky...
    auto pointer_location = reinterpret_cast<std::uintptr_t>(definition);
    std::stringstream ss;
    ss << pointer_location;

    Symbol *symbol = new Symbol(ss.str());
    symbol->nameSpace = new Namespace(m_current);
    functionSymbol->nameSpace->insert(definition, symbol);

    Namespace *oldNamespace = m_current;
    m_current = symbol->nameSpace;

    for (auto parameter : definition->type_parameters) {
        Symbol *sym = new Symbol(parameter->name);
        sym->type = new Types::Parameter(parameter->name);
        m_current->insert(definition, sym);
    }

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->code->accept(this);

    m_current = oldNamespace;
}

void Builder::visit(AST::TypeDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->name);
    m_current->insert(definition, symbol);

    symbol->nameSpace = new Namespace(m_current);

    Namespace *oldNamespace = m_current;
    m_current = symbol->nameSpace;

    for (auto parameter : definition->parameters) {
        Symbol *sym = new Symbol(parameter->name);
        sym->type = new Types::Parameter(parameter->name);
        m_current->insert(definition, sym);
    }

    if (definition->alias) {
        // do nothing
    } else {
        for (auto field : definition->fields) {
            Symbol *sym = new Symbol(field->name->name);
            m_current->insert(field, sym);
        }
    }

    m_current = oldNamespace;
}

void Builder::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Builder::visit(AST::ExpressionStatement *statement) {

}

void Builder::visit(AST::ImportStatement *statement) {

}

void Builder::visit(AST::SourceFile *module) {
    module->code->accept(this);
}
