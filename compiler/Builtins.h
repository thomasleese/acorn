//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_BUILTINS_H
#define JET_BUILTINS_H

namespace SymbolTable {
    class Namespace;
}

namespace Builtins {

    void fill_symbol_table(SymbolTable::Namespace *table);

};

#endif //JET_BUILTINS_H
