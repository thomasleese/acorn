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

Namespace* Namespace::clone() const {
    Namespace *new_namespace = new Namespace(m_parent);
    for (auto entry : m_symbols) {
        new_namespace->m_symbols[entry.first] = entry.second->clone();
    }
    return new_namespace;
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

Symbol* Symbol::clone() const {
    auto new_symbol = new Symbol(this->name);
    if (this->nameSpace) {
        new_symbol->nameSpace = this->nameSpace->clone();
    }
    return new_symbol;
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

Symbol *Builder::add_builtin_function(std::string name) {
    auto symbol = add_builtin_symbol(name, new types::Function());
    symbol->nameSpace = new Namespace(scope());
    return symbol;
}

Symbol *Builder::add_builtin_method(symboltable::Symbol *function, types::Method *method) {
    auto symbol = new Symbol(method->mangled_name());
    symbol->type = method;
    symbol->nameSpace = new Namespace(function->nameSpace);
    function->nameSpace->insert(nullptr, nullptr, symbol);

    auto function_type = static_cast<types::Function *>(function->type);
    function_type->add_method(method);

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

void Builder::add_builtin_variables() {
    add_builtin_symbol("nil", new types::Void());
    add_builtin_symbol("true", new types::Boolean());
    add_builtin_symbol("false", new types::Boolean());
}

void Builder::add_builtin_functions() {
    auto not_ = add_builtin_function("not");
    add_builtin_method(not_, new types::Method(new types::Boolean(), new types::Boolean()));

    auto multiplication = add_builtin_function("*");
    add_builtin_method(multiplication, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    auto addition = add_builtin_function("+");
    add_builtin_method(addition, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));
    add_builtin_method(addition, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::UnsignedInteger(64)));
    add_builtin_method(addition, new types::Method(new types::Float(64), new types::Float(64), new types::Float(64)));

    auto subtraction = add_builtin_function("-");
    add_builtin_method(subtraction, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    auto equality = add_builtin_function("==");
    add_builtin_method(equality, new types::Method(new types::Boolean(), new types::Boolean(), new types::Boolean()));
    add_builtin_method(equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));
    add_builtin_method(equality, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::Boolean()));

    auto not_equality = add_builtin_function("!=");
    add_builtin_method(not_equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    auto less_than = add_builtin_function("<");
    add_builtin_method(less_than, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    auto gte = add_builtin_function(">=");
    add_builtin_method(gte, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    auto to_integer = add_builtin_function("to_integer");
    add_builtin_method(to_integer, new types::Method(new types::Float(64), new types::Integer(64)));

    auto to_float = add_builtin_function("to_float");
    add_builtin_method(to_float, new types::Method(new types::Integer(64), new types::Float(64)));

    auto getindex = add_builtin_function("getindex");
    auto pc = new types::ParameterType();
    auto method = new types::Method(new types::UnsafePointer(new types::Parameter(pc)), new types::Integer(64), new types::Parameter(pc));
    method->set_is_generic(true);
    add_builtin_method(getindex, method);

    auto setindex = add_builtin_function("setindex");
    pc = new types::ParameterType();
    method = new types::Method(new types::UnsafePointer(new types::Parameter(pc)), new types::Integer(64), new types::Parameter(pc), new types::Void());
    method->set_is_generic(true);
    add_builtin_method(setindex, method);

    auto sizeof_ = add_builtin_function("sizeof");
    pc = new types::ParameterType();
    method = new types::Method(pc, new types::Integer(64));
    method->set_is_generic(true);
    add_builtin_method(sizeof_, method);

    auto strideof_ = add_builtin_function("strideof");
    pc = new types::ParameterType();
    method = new types::Method(pc, new types::Integer(64));
    method->set_is_generic(true);
    add_builtin_method(strideof_, method);
}

void Builder::add_builtins() {
    add_builtin_types();
    add_builtin_variables();
    add_builtin_functions();
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

void Builder::visit(ast::TupleLiteral *expression) {

}

void Builder::visit(ast::Call *expression) {

}

void Builder::visit(ast::CCall *expression) {

}

void Builder::visit(ast::Cast *expression) {

}

void Builder::visit(ast::Assignment *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
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
    Symbol *symbol = new Symbol(parameter->name->value());
    scope()->insert(this, parameter, symbol);
}

void Builder::visit(ast::Let *definition) {
    definition->assignment->accept(this);

    if (definition->has_body()) {
        definition->body()->accept(this);
    }
}

void Builder::visit(ast::FunctionDefinition *definition) {
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

    for (auto parameter : definition->parameters) {
        parameter->accept(this);
    }

    definition->body->accept(this);

    pop_scope();
}

void Builder::visit(ast::TypeDefinition *definition) {
    Symbol *symbol = new Symbol(definition->name()->value());
    scope()->insert(this, definition, symbol);

    symbol->nameSpace = new Namespace(scope());

    push_scope(symbol);

    for (auto parameter : definition->name()->parameters()) {
        Symbol *sym = new Symbol(parameter->value());
        sym->type = new types::ParameterType();
        scope()->insert(this, definition, sym);
    }

    if (definition->alias) {
        // do nothing
    } else {
        for (auto name : definition->field_names) {
            Symbol *sym = new Symbol(name->value());
            scope()->insert(this, name, sym);
        }
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
