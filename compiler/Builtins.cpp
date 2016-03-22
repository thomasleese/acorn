//
// Created by Thomas Leese on 18/03/2016.
//

#include "SymbolTable.h"
#include "Types.h"

#include "Builtins.h"

void add_base_type(SymbolTable::Namespace *table, std::string name, Types::Type *type) {
    SymbolTable::Symbol *symbol = new SymbolTable::Symbol(name);
    symbol->type = new Types::TypeType(type);
    table->insert(0, symbol);
}

void Builtins::fill_symbol_table(SymbolTable::Namespace *table) {
    Types::Void *voidType = new Types::Void();

    add_base_type(table, "Any", new Types::Any());
    add_base_type(table, "Void", voidType);
    add_base_type(table, "Boolean", new Types::Boolean());
    add_base_type(table, "Integer8", new Types::Integer8());
    add_base_type(table, "Integer16", new Types::Integer16());
    add_base_type(table, "Integer32", new Types::Integer32());
    add_base_type(table, "Integer64", new Types::Integer64());
    add_base_type(table, "Integer128", new Types::Integer128());
    add_base_type(table, "Float16", new Types::Float16());
    add_base_type(table, "Float32", new Types::Float32());
    add_base_type(table, "Float64", new Types::Float64());
    add_base_type(table, "Float128", new Types::Float128());
    add_base_type(table, "Function", new Types::Function());
    add_base_type(table, "Sequence", new Types::Void());
    add_base_type(table, "Product", new Types::Product());
    add_base_type(table, "Union", new Types::Union());

    SymbolTable::Symbol *symbol = new SymbolTable::Symbol("Nothing");
    symbol->type = voidType;
    table->insert(0, symbol);
}
