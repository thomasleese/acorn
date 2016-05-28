//
// Created by Thomas Leese on 15/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "ast/nodes.h"
#include "errors.h"
#include "parsing/lexer.h"
#include "typing/types.h"

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
    return dynamic_cast<types::TypeType *>(this->type) != nullptr;
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
    m_scope.push_back(m_root);

    add_builtins();
}

bool Builder::isAtRoot() const {
    return m_root == m_scope.back();
}

Namespace *Builder::rootNamespace() {
    return m_root;
}

Symbol *Builder::add_builtin_symbol(std::string name, types::Type *type) {
    auto symbol = new Symbol(name);
    symbol->type = type;
    symbol->is_builtin = true;
    m_scope.back()->insert(nullptr, nullptr, symbol);
    return symbol;
}

Symbol *Builder::add_builtin_function(std::string name) {
    auto symbol = add_builtin_symbol(name, new types::Function());
    symbol->nameSpace = new Namespace(m_scope.back());
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
    add_builtin_symbol("Boolean", new types::BooleanType());
    add_builtin_symbol("Integer8", new types::IntegerType(8));
    add_builtin_symbol("Integer16", new types::IntegerType(16));
    add_builtin_symbol("Integer32", new types::IntegerType(32));
    add_builtin_symbol("Integer64", new types::IntegerType(64));
    add_builtin_symbol("Integer128", new types::IntegerType(128));
    add_builtin_symbol("UnsignedInteger8", new types::UnsignedIntegerType(8));
    add_builtin_symbol("UnsignedInteger16", new types::UnsignedIntegerType(16));
    add_builtin_symbol("UnsignedInteger32", new types::UnsignedIntegerType(32));
    add_builtin_symbol("UnsignedInteger64", new types::UnsignedIntegerType(64));
    add_builtin_symbol("UnsignedInteger128", new types::UnsignedIntegerType(128));
    add_builtin_symbol("Float16", new types::FloatType(16));
    add_builtin_symbol("Float32", new types::FloatType(32));
    add_builtin_symbol("Float64", new types::FloatType(64));
    add_builtin_symbol("Float128", new types::FloatType(128));
    add_builtin_symbol("UnsafePointer", new types::UnsafePointerType());
    add_builtin_symbol("Function", new types::FunctionType());
    add_builtin_symbol("Tuple", new types::TupleType());
    add_builtin_symbol("Type", new types::TypeDescriptionType());
}

void Builder::add_builtin_variables() {
    add_builtin_symbol("Nothing", new types::Void());
    add_builtin_symbol("True", new types::Boolean());
    add_builtin_symbol("False", new types::Boolean());
}

void Builder::add_builtin_functions() {
    auto not_ = add_builtin_function("not");
    add_builtin_method(not_, new types::Method(new types::Boolean(), new types::Boolean()));

    symboltable::Symbol *multiplication = add_builtin_function("*");
    add_builtin_method(multiplication, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    symboltable::Symbol *addition = add_builtin_function("+");
    add_builtin_method(addition, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));
    add_builtin_method(addition, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::UnsignedInteger(64)));
    add_builtin_method(addition, new types::Method(new types::Float(64), new types::Float(64), new types::Float(64)));

    symboltable::Symbol *subtraction = add_builtin_function("-");
    add_builtin_method(subtraction, new types::Method(new types::Integer(64), new types::Integer(64), new types::Integer(64)));

    symboltable::Symbol *equality = add_builtin_function("==");
    add_builtin_method(equality, new types::Method(new types::Boolean(), new types::Boolean(), new types::Boolean()));
    add_builtin_method(equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));
    add_builtin_method(equality, new types::Method(new types::UnsignedInteger(64), new types::UnsignedInteger(64), new types::Boolean()));

    symboltable::Symbol *not_equality = add_builtin_function("!=");
    add_builtin_method(not_equality, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    symboltable::Symbol *less_than = add_builtin_function("<");
    add_builtin_method(less_than, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    auto gte = add_builtin_function(">=");
    add_builtin_method(gte, new types::Method(new types::Integer(64), new types::Integer(64), new types::Boolean()));

    symboltable::Symbol *to_integer = add_builtin_function("to_integer");
    add_builtin_method(to_integer, new types::Method(new types::Float(64), new types::Integer(64)));

    symboltable::Symbol *to_float = add_builtin_function("to_float");
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

void Builder::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Builder::visit(ast::Identifier *identifier) {

}

void Builder::visit(ast::VariableDeclaration *node) {
    assert(!node->name()->has_parameters());

    Symbol *symbol = new Symbol(node->name()->value);
    m_scope.back()->insert(this, node, symbol);
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
    expression->code()->accept(this);
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

void Builder::visit(ast::Parameter *parameter) {
    Symbol *symbol = new Symbol(parameter->name->value);
    m_scope.back()->insert(this, parameter, symbol);
}

void Builder::visit(ast::VariableDefinition *definition) {
    definition->assignment->accept(this);
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
        sym->type = new types::ParameterType();
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
        sym->type = new types::ParameterType();
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

void Builder::visit(ast::ProtocolDefinition *definition) {
    auto symbol = new Symbol(definition->name->value);
    m_scope.back()->insert(this, definition, symbol);

    symbol->nameSpace = new Namespace(m_scope.back());

    m_scope.push_back(symbol->nameSpace);

    auto self_symbol = new Symbol("Self");
    self_symbol->type = new types::ParameterType();
    m_scope.back()->insert(this, definition, self_symbol);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        sym->type = new types::ParameterType();
        m_scope.back()->insert(this, definition, sym);
    }

    for (auto method : definition->methods()) {
        method->accept(this);
    }

    m_scope.pop_back();
}

void Builder::visit(ast::EnumDefinition *definition) {
    auto symbol = new Symbol(definition->name->value);
    m_scope.back()->insert(this, definition, symbol);

    symbol->nameSpace = new Namespace(m_scope.back());

    m_scope.push_back(symbol->nameSpace);

    auto self_symbol = new Symbol("Self");
    self_symbol->type = new types::ParameterType();
    m_scope.back()->insert(this, definition, self_symbol);

    for (auto parameter : definition->name->parameters) {
        Symbol *sym = new Symbol(parameter->value);
        sym->type = new types::ParameterType();
        m_scope.back()->insert(this, definition, sym);
    }

    for (auto element : definition->elements()) {
        Symbol *sym = new Symbol(element->name()->value);
        m_scope.back()->insert(this, element, sym);
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
