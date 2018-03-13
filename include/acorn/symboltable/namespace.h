#pragma once

#include <map>
#include <string>
#include <vector>

namespace acorn {

    namespace ast {
        class Node;
        class Name;
        class TypeName;
        class DeclName;
        class ParamName;
    }

    namespace diagnostics {
        class Reporter;
    }

    namespace typesystem {
        class Type;
        class Method;
    }

}

namespace acorn::symboltable {

    class Symbol;

    class Namespace {
    public:
        explicit Namespace(Namespace *parent);
        ~Namespace();

        bool has(std::string name, bool follow_parents = true) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Node *current_node, std::string name) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::Name *name) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::TypeName *name) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::DeclName *name) const;
        Symbol *lookup(diagnostics::Reporter *diagnostics, ast::ParamName *name) const;
        Symbol *lookup_by_node(diagnostics::Reporter *diagnostics, ast::Node *node) const;
        void insert(diagnostics::Reporter *diagnostics, ast::Node *current_node, std::unique_ptr<Symbol> symbol);
        void rename(diagnostics::Reporter *diagnostics, Symbol *symbol, std::string new_name);
        unsigned long size() const;
        std::vector<Symbol *> symbols() const;
        bool is_root() const;

        std::string to_string(int indent = 0) const;

    private:
        Namespace *m_parent;
        std::map<std::string, std::unique_ptr<Symbol>> m_symbols;
    };

}
