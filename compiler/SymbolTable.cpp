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
    return lookup(identifier, identifier->value);
}

Symbol *Namespace::lookup_by_node(AST::Node *node) const {
    for (auto entry : m_symbols) {
        if (entry.second->node == node) {
            return entry.second;
        }
    }

    if (m_parent) {
        return m_parent->lookup_by_node(node);
    } else {
        throw Errors::UndefinedError(node, node->token->lexeme);
    }
}

void Namespace::insert(AST::Node *currentNode, Symbol *symbol) {
    symbol->node = currentNode;

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
    insert(symbol->node, symbol);
}

unsigned long Namespace::size() const {
    return m_symbols.size();
}

std::vector<Symbol *> Namespace::symbols() const {
    std::vector<Symbol *> symbols;
    for (auto entry : m_symbols) {
        symbols.push_back(entry.second);
    }
    return symbols;
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

Namespace* Namespace::clone() const {
    Namespace *new_namespace = new Namespace(m_parent);
    for (auto entry : m_symbols) {
        new_namespace->m_symbols[entry.first] = entry.second->clone();
    }
    return new_namespace;
}

Symbol::Symbol(std::string name) {
    this->name = name;
    this->type = nullptr;
    this->value = nullptr;
    this->nameSpace = nullptr;
    this->is_builtin = false;
}

bool Symbol::is_function() const {
    return dynamic_cast<Types::Function *>(this->type) != nullptr && this->node == nullptr;
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

Symbol* Symbol::clone() const {
    auto new_symbol = new Symbol(this->name);
    if (this->nameSpace) {
        new_symbol->nameSpace = this->nameSpace->clone();
    }
    return new_symbol;
}

Builder::Builder() {
    m_root = new Namespace(nullptr);
    Builtins::fill_symbol_table(m_root);

    m_scope.push_back(m_root);
}

bool Builder::isAtRoot() const {
    return m_root == m_scope.back();
}

Namespace *Builder::rootNamespace() {
    return m_root;
}

void Builder::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Builder::visit(AST::Identifier *identifier) {

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

void Builder::visit(AST::Cast *expression) {

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
    Symbol *symbol = new Symbol(parameter->name->value);
    m_scope.back()->insert(parameter, symbol);
}

void Builder::visit(AST::VariableDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->value);
    m_scope.back()->insert(definition, symbol);
}

void Builder::visit(AST::FunctionDefinition *definition) {
    Symbol *functionSymbol;
    if (m_scope.back()->has(definition->name->value, false)) {
        // we don't want to look in any parent scope when we're
        // defining a new function; it should follow the notion of
        // variables, i.e. we are hiding the previous binding
        functionSymbol = m_scope.back()->lookup(definition->name);
    } else {
        functionSymbol = new Symbol(definition->name->value);
        functionSymbol->type = new Types::Function();
        functionSymbol->nameSpace = new Namespace(m_scope.back());
        m_scope.back()->insert(definition, functionSymbol);
        functionSymbol->node = nullptr;  // explicit no node for function symbols
    }

    // this is really hacky...
    auto pointer_location = reinterpret_cast<std::uintptr_t>(definition);
    std::stringstream ss;
    ss << pointer_location;

    Symbol *symbol = new Symbol(ss.str());
    symbol->nameSpace = new Namespace(m_scope.back());
    functionSymbol->nameSpace->insert(definition, symbol);

    m_scope.push_back(symbol->nameSpace);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        m_scope.back()->insert(definition, sym);
    }

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->code->accept(this);

    m_scope.pop_back();
}

void Builder::visit(AST::TypeDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->value);
    m_scope.back()->insert(definition, symbol);

    symbol->nameSpace = new Namespace(m_scope.back());

    m_scope.push_back(symbol->nameSpace);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        m_scope.back()->insert(definition, sym);
    }

    if (definition->alias) {
        // do nothing
    } else {
        for (auto field : definition->fields) {
            Symbol *sym = new Symbol(field->name->value);
            m_scope.back()->insert(field, sym);
        }
    }

    m_scope.pop_back();
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
