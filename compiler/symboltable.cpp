//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "ast/nodes.h"
#include "Lexer.h"
#include "Errors.h"
#include "builtins.h"
#include "Types.h"

#include "SymbolTable.h"

using namespace acorn;
using namespace acorn::symboltable;

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

Symbol *Namespace::lookup(compiler::Pass *pass, ast::Node *currentNode, std::string name) const {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        if (m_parent) {
            return m_parent->lookup(pass, currentNode, name);
        } else {
            pass->push_error(new errors::UndefinedError(currentNode, name));
            return nullptr;
        }
    }

    return it->second;
}

Symbol *Namespace::lookup(compiler::Pass *pass, ast::Identifier *identifier) const {
    return lookup(pass, identifier, identifier->value);
}

Symbol *Namespace::lookup_by_node(compiler::Pass *pass, ast::Node *node) const {
    for (auto entry : m_symbols) {
        if (entry.second->node == node) {
            return entry.second;
        }
    }

    if (m_parent) {
        return m_parent->lookup_by_node(pass, node);
    } else {
        pass->push_error(new errors::UndefinedError(node, node->token->lexeme));
        return nullptr;
    }
}

void Namespace::insert(compiler::Pass *pass, ast::Node *currentNode, Symbol *symbol) {
    symbol->node = currentNode;

    auto it = m_symbols.find(symbol->name);
    if (it != m_symbols.end()) {
        pass->push_error(new errors::RedefinedError(currentNode, symbol->name));
    }

    m_symbols[symbol->name] = symbol;
}

void Namespace::rename(compiler::Pass *pass, Symbol *symbol, std::string new_name) {
    auto it = m_symbols.find(symbol->name);
    assert(it != m_symbols.end());

    m_symbols.erase(it);
    symbol->name = new_name;
    insert(pass, symbol->node, symbol);
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
    return dynamic_cast<types::Function *>(this->type) != nullptr && this->node == nullptr;
}

bool Symbol::is_type() const {
    return dynamic_cast<types::Constructor *>(this->type) != nullptr;
}

bool Symbol::is_variable() const {
    return dynamic_cast<ast::VariableDefinition *>(this->node) != nullptr;
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
    builtins::fill_symbol_table(m_root);

    m_scope.push_back(m_root);
}

bool Builder::isAtRoot() const {
    return m_root == m_scope.back();
}

Namespace *Builder::rootNamespace() {
    return m_root;
}

void Builder::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Builder::visit(ast::Identifier *identifier) {

}

void Builder::visit(ast::BooleanLiteral *boolean) {

}

void Builder::visit(ast::IntegerLiteral *expression) {

}

void Builder::visit(ast::FloatLiteral *expression) {

}

void Builder::visit(ast::ImaginaryLiteral *imaginary) {

}

void Builder::visit(ast::StringLiteral *expression) {

}

void Builder::visit(ast::SequenceLiteral *sequence) {

}

void Builder::visit(ast::MappingLiteral *mapping) {

}

void Builder::visit(ast::RecordLiteral *expression) {

}

void Builder::visit(ast::Call *expression) {

}

void Builder::visit(ast::CCall *expression) {

}

void Builder::visit(ast::Cast *expression) {

}

void Builder::visit(ast::Assignment *expression) {

}

void Builder::visit(ast::Selector *expression) {

}

void Builder::visit(ast::Comma *expression) {

}

void Builder::visit(ast::While *expression) {

}

void Builder::visit(ast::For *expression) {

}

void Builder::visit(ast::If *expression) {
    expression->condition->accept(this);

    expression->trueCode->accept(this);

    if (expression->falseCode) {
        expression->falseCode->accept(this);
    }
}

void Builder::visit(ast::Return *expression) {

}

void Builder::visit(ast::Spawn *expression) {

}

void Builder::visit(ast::Sizeof *expression) {

}

void Builder::visit(ast::Strideof *expression) {

}

void Builder::visit(ast::Parameter *parameter) {
    Symbol *symbol = new Symbol(parameter->name->value);
    m_scope.back()->insert(this, parameter, symbol);
}

void Builder::visit(ast::VariableDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->value);
    m_scope.back()->insert(this, definition, symbol);

    symbol->nameSpace = new Namespace(m_scope.back());

    m_scope.push_back(symbol->nameSpace);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        sym->type = new types::Parameter();
        m_scope.back()->insert(this, definition, sym);
    }

    m_scope.pop_back();
}

void Builder::visit(ast::FunctionDefinition *definition) {
    Symbol *functionSymbol;
    if (m_scope.back()->has(definition->name->value, false)) {
        // we don't want to look in any parent scope when we're
        // defining a new function; it should follow the notion of
        // variables, i.e. we are hiding the previous binding
        functionSymbol = m_scope.back()->lookup(this, definition->name);
    } else {
        functionSymbol = new Symbol(definition->name->value);
        functionSymbol->type = new types::Function();
        functionSymbol->nameSpace = new Namespace(m_scope.back());
        m_scope.back()->insert(this, definition, functionSymbol);
        functionSymbol->node = nullptr;  // explicit no node for function symbols
    }

    // this is really hacky...
    auto pointer_location = reinterpret_cast<std::uintptr_t>(definition);
    std::stringstream ss;
    ss << pointer_location;

    Symbol *symbol = new Symbol(ss.str());
    symbol->nameSpace = new Namespace(m_scope.back());
    functionSymbol->nameSpace->insert(this, definition, symbol);

    m_scope.push_back(symbol->nameSpace);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        sym->type = new types::Parameter();
        m_scope.back()->insert(this, definition, sym);
    }

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->code->accept(this);

    m_scope.pop_back();
}

void Builder::visit(ast::TypeDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name->value);
    m_scope.back()->insert(this, definition, symbol);

    symbol->nameSpace = new Namespace(m_scope.back());

    m_scope.push_back(symbol->nameSpace);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        sym->type = new types::Parameter();
        m_scope.back()->insert(this, definition, sym);
    }

    if (definition->alias) {
        // do nothing
    } else {
        for (auto name : definition->field_names) {
            Symbol *sym = new Symbol(name->value);
            m_scope.back()->insert(this, name, sym);
        }
    }

    m_scope.pop_back();
}

void Builder::visit(ast::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Builder::visit(ast::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Builder::visit(ast::ImportStatement *statement) {

}

void Builder::visit(ast::SourceFile *module) {
    module->code->accept(this);
}
