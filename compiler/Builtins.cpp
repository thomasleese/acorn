//
// Created by Thomas Leese on 18/03/2016.
//

#include "SymbolTable.h"
#include "Types.h"

#include "Builtins.h"

void add_base_type(SymbolTable::Namespace *table, std::string name, Types::Type *type) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    symbol->type = type;
    table->insert(0, symbol);
}

void Builtins::fill_symbol_table(SymbolTable::Namespace *table) {
    add_base_type(table, "Any", new Types::AnyConstructor());
    add_base_type(table, "Void", new Types::VoidConstructor());
    add_base_type(table, "Boolean", new Types::BooleanConstructor());
    add_base_type(table, "Integer8", new Types::IntegerConstructor(8));
    add_base_type(table, "Integer16", new Types::IntegerConstructor(16));
    add_base_type(table, "Integer32", new Types::IntegerConstructor(32));
    add_base_type(table, "Integer64", new Types::IntegerConstructor(64));
    add_base_type(table, "Integer128", new Types::IntegerConstructor(128));
    add_base_type(table, "Float16", new Types::FloatConstructor(16));
    add_base_type(table, "Float32", new Types::FloatConstructor(32));
    add_base_type(table, "Float64", new Types::FloatConstructor(64));
    add_base_type(table, "Float128", new Types::FloatConstructor(128));
    add_base_type(table, "Sequence", new Types::SequenceConstructor());
    add_base_type(table, "Function", new Types::FunctionConstructor());
    add_base_type(table, "Union", new Types::UnionConstructor());

    SymbolTable::Symbol *symbol = new SymbolTable::Symbol("Nothing");
    symbol->type = new Types::Void();
    table->insert(nullptr, symbol);

    SymbolTable::Symbol *symbol2 = new SymbolTable::Symbol("_debug_print_");
    std::map<std::string,Types::Type *> p;
    p["x"] = new Types::Integer(64);
    Types::Method *method = new Types::Method(p, new Types::Void());
    Types::Function *function = new Types::Function();
    function->add_method(method);
    symbol2->type = function;
    table->insert(nullptr, symbol2);
}
