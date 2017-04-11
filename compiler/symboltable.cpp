//
// Created by Thomas Leese on 15/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "ast.h"
#include "diagnostics.h"
#include "lexer.h"
#include "types.h"

#include "symboltable.h"

using namespace acorn;
using namespace acorn::diagnostics;
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

Symbol *Namespace::lookup(Reporter *diagnostics, ast::Node *currentNode, std::string name) const {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        if (m_parent) {
            return m_parent->lookup(diagnostics, currentNode, name);
        } else {
            diagnostics->report(UndefinedError(currentNode, name));
            return nullptr;
        }
    }

    return it->second;
}

Symbol *Namespace::lookup(Reporter *diagnostics, ast::Name *identifier) const {
    return lookup(diagnostics, identifier, identifier->value());
}

Symbol *Namespace::lookup_by_node(Reporter *diagnostics, ast::Node *node) const {
    for (auto entry : m_symbols) {
        if (entry.second->node == node) {
            return entry.second;
        }
    }

    if (m_parent) {
        return m_parent->lookup_by_node(diagnostics, node);
    } else {
        diagnostics->report(UndefinedError(node, node->token().lexeme));
        return nullptr;
    }
}

void Namespace::insert(Reporter *diagnostics, ast::Node *currentNode, Symbol *symbol) {
    symbol->node = currentNode;

    auto it = m_symbols.find(symbol->name);
    if (it != m_symbols.end()) {
        diagnostics->report(RedefinedError(currentNode, symbol->name));
    }

    m_symbols[symbol->name] = symbol;
}

void Namespace::rename(Reporter *diagnostics, Symbol *symbol, std::string new_name) {
    auto it = m_symbols.find(symbol->name);
    assert(it != m_symbols.end());

    m_symbols.erase(it);
    symbol->name = new_name;
    insert(diagnostics, symbol->node, symbol);
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

std::string Namespace::to_string(int indent) const {
    std::stringstream ss;

    std::string gap = "";
    for (int i = 0; i < indent; i++) {
        gap += " ";
    }

    ss << gap << "{\n";

    for (auto it = m_symbols.begin(); it != m_symbols.end(); it++) {
        auto symbol = it->second;
        ss << gap << " " << symbol->to_string(indent + 1) << "\n";
    }

    ss << gap << "}";

    return ss.str();
}

Symbol::Symbol(std::string name) : type(nullptr), value(nullptr), nameSpace(nullptr), node(nullptr), is_builtin(false) {
    this->name = name;
}

Symbol::Symbol(ast::Name *name) : Symbol(name->value()) {

}

bool Symbol::is_function() const {
    return dynamic_cast<types::Function *>(this->type) != nullptr && this->node == nullptr;
}

bool Symbol::is_type() const {
    return dynamic_cast<types::TypeType *>(this->type) != nullptr;
}

bool Symbol::is_variable() const {
    return dynamic_cast<ast::Let *>(this->node) != nullptr;
}

void Symbol::copy_type_from(ast::Expression *expression) {
    // TODO check type is not null?
    this->type = expression->type();
}

std::string Symbol::to_string(int indent) const {
    std::stringstream ss;
    ss << this->name << " (Node: " << this->node << ") (Value: " << this->value << ")";

    if (this->type) {
        ss << ": " << this->type->name();
    }

    if (this->nameSpace) {
        ss << " " << this->nameSpace->to_string(indent + 1);
    }

    return ss.str();
}

void ScopeFollower::push_scope(symboltable::Symbol *symbol) {
    push_scope(symbol->nameSpace);
}

void ScopeFollower::push_scope(symboltable::Namespace *name_space) {
    m_scope.push_back(name_space);
}

void ScopeFollower::pop_scope() {
    m_scope.pop_back();
}

symboltable::Namespace *ScopeFollower::scope() const {
    return m_scope.back();
}

Builder::Builder() {
    m_root = new Namespace(nullptr);
    push_scope(m_root);

    add_builtins();
}

bool Builder::is_at_root() const {
    return m_root == scope();
}

Namespace *Builder::root_namespace() {
    return m_root;
}

Symbol *Builder::add_builtin_symbol(std::string name, types::Type *type) {
    auto symbol = new Symbol(name);
    symbol->type = type;
    symbol->is_builtin = true;
    scope()->insert(nullptr, nullptr, symbol);
    return symbol;
}

void Builder::add_builtin_types() {
    add_builtin_symbol("Void", new types::VoidType());
    add_builtin_symbol("Bool", new types::BooleanType());
    add_builtin_symbol("Int8", new types::IntegerType(8));
    add_builtin_symbol("Int16", new types::IntegerType(16));
    add_builtin_symbol("Int32", new types::IntegerType(32));
    add_builtin_symbol("Int64", new types::IntegerType(64));
    add_builtin_symbol("Int128", new types::IntegerType(128));
    add_builtin_symbol("UInt8", new types::UnsignedIntegerType(8));
    add_builtin_symbol("UInt16", new types::UnsignedIntegerType(16));
    add_builtin_symbol("UInt32", new types::UnsignedIntegerType(32));
    add_builtin_symbol("UInt64", new types::UnsignedIntegerType(64));
    add_builtin_symbol("UInt128", new types::UnsignedIntegerType(128));
    add_builtin_symbol("Float16", new types::FloatType(16));
    add_builtin_symbol("Float32", new types::FloatType(32));
    add_builtin_symbol("Float64", new types::FloatType(64));
    add_builtin_symbol("Float128", new types::FloatType(128));
    add_builtin_symbol("UnsafePointer", new types::UnsafePointerType());
    add_builtin_symbol("Function", new types::FunctionType());
    add_builtin_symbol("Method", new types::MethodType());
    add_builtin_symbol("Tuple", new types::TupleType());
    add_builtin_symbol("Type", new types::TypeDescriptionType());
}

void Builder::add_builtins() {
    add_builtin_types();
}

void Builder::visit(ast::Block *block) {
    for (auto expression : block->expressions()) {
        expression->accept(this);
    }
}

void Builder::visit(ast::Name *identifier) {

}

void Builder::visit(ast::VariableDeclaration *node) {
    assert(!node->name()->has_parameters());

    Symbol *symbol = new Symbol(node->name()->value());
    scope()->insert(this, node, symbol);
}

void Builder::visit(ast::Int *expression) {

}

void Builder::visit(ast::Float *expression) {

}

void Builder::visit(ast::Complex *imaginary) {

}

void Builder::visit(ast::String *expression) {

}

void Builder::visit(ast::List *sequence) {

}

void Builder::visit(ast::Dictionary *mapping) {

}

void Builder::visit(ast::Tuple *expression) {

}

void Builder::visit(ast::Call *expression) {

}

void Builder::visit(ast::CCall *expression) {

}

void Builder::visit(ast::Cast *expression) {

}

void Builder::visit(ast::Assignment *node) {
    node->lhs->accept(this);

    if (!node->builtin()) {
        node->rhs->accept(this);
    }
}

void Builder::visit(ast::Selector *expression) {

}

void Builder::visit(ast::While *expression) {
    expression->condition()->accept(this);
    expression->body()->accept(this);
}

void Builder::visit(ast::If *expression) {
    expression->condition->accept(this);

    expression->true_case->accept(this);

    if (expression->false_case) {
        expression->false_case->accept(this);
    }
}

void Builder::visit(ast::Return *expression) {

}

void Builder::visit(ast::Spawn *expression) {

}

void Builder::visit(ast::Switch *expression) {
    for (auto entry : expression->cases()) {
        entry->condition()->accept(this);

        if (entry->assignment()) {
            entry->assignment()->accept(this);
        }
    }

    if (expression->default_case()) {
        expression->default_case()->accept(this);
    }
}

void Builder::visit(ast::Parameter *parameter) {
    auto symbol = new Symbol(parameter->name()->value());
    scope()->insert(this, parameter, symbol);
}

void Builder::visit(ast::Let *definition) {
    definition->assignment->accept(this);

    if (definition->has_body()) {
        definition->body()->accept(this);
    }
}

void Builder::visit(ast::Def *definition) {
    Symbol *functionSymbol;
    if (scope()->has(definition->name()->value(), false)) {
        // we don't want to look in any parent scope when we're
        // defining a new function; it should follow the notion of
        // variables, i.e. we are hiding the previous binding
        functionSymbol = scope()->lookup(this, definition->name());
    } else {
        functionSymbol = new Symbol(definition->name()->value());
        functionSymbol->type = new types::Function();
        functionSymbol->nameSpace = new Namespace(scope());
        scope()->insert(this, definition, functionSymbol);
        functionSymbol->node = nullptr;  // explicit no node for function symbols
    }

    // this is really hacky...
    auto pointer_location = reinterpret_cast<std::uintptr_t>(definition);
    std::stringstream ss;
    ss << pointer_location;

    Symbol *symbol = new Symbol(ss.str());
    symbol->nameSpace = new Namespace(scope());
    functionSymbol->nameSpace->insert(this, definition, symbol);

    push_scope(symbol);

    for (auto parameter : definition->name()->parameters()) {
        Symbol *sym = new Symbol(parameter->value());
        sym->type = new types::ParameterType();
        scope()->insert(this, parameter, sym);
    }

    for (auto parameter : definition->parameters()) {
        parameter->accept(this);
    }

    if (!definition->builtin()) {
        definition->body()->accept(this);
    }

    pop_scope();
}

void Builder::visit(ast::Type *definition) {
    Symbol *symbol = new Symbol(definition->name()->value());
    scope()->insert(this, definition, symbol);

    symbol->nameSpace = new Namespace(scope());

    push_scope(symbol);

    for (auto parameter : definition->name()->parameters()) {
        Symbol *sym = new Symbol(parameter->value());
        sym->type = new types::ParameterType();
        scope()->insert(this, parameter, sym);
    }

    if (definition->alias) {
        // do nothing
    } else {
        auto constructor_symbol = new Symbol("new");
        scope()->insert(this, definition, constructor_symbol);
    }

    pop_scope();
}

void Builder::visit(ast::Module *module) {
    symboltable::Symbol *symbol;
    if (scope()->has(module->name()->value())) {
        symbol = scope()->lookup(this, module->name());
    } else {
        symbol = new Symbol(module->name());
        symbol->nameSpace = new Namespace(scope());
        scope()->insert(this, module, symbol);
    }

    push_scope(symbol);
    module->body()->accept(this);
    pop_scope();
}

void Builder::visit(ast::Import *statement) {

}

void Builder::visit(ast::SourceFile *module) {
    module->code->accept(this);
}
