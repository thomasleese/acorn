//
// Created by Thomas Leese on 15/03/2016.
//

#pragma once

#include <string>

namespace llvm {
    class Value;
}

namespace acorn {

    namespace ast {
        class Node;
    }

    namespace typesystem {
        class Type;
        class Method;
    }

}

namespace acorn::symboltable {

    class Namespace;

    class Symbol {
    public:
        Symbol(std::string name, bool builtin);
        Symbol(ast::Name *name, bool builtin);

        std::string name() const { return m_name; }
        void set_name(std::string name) { m_name = name; }

        bool builtin() const { return m_builtin; }

        bool has_type() const { return m_type != nullptr; }
        typesystem::Type *type() const { return m_type; }
        void set_type(typesystem::Type *type) { m_type = type; }

        bool has_llvm_value() const { return m_llvm_value != nullptr; }
        llvm::Value *llvm_value() const { return m_llvm_value; }
        void set_llvm_value(llvm::Value *value) { m_llvm_value = value; }

        Namespace *scope() const { return m_scope.get(); }
        void initialise_scope(Namespace *parent);

        ast::Node *node() const { return m_node; }
        void initialise_node(ast::Node *node);

        bool is_function() const;
        bool is_variable() const;
        bool is_type() const;

        void copy_type_from(ast::Expression *expression);

        std::string to_string(int indent = 0) const;

    private:
        std::string m_name;
        bool m_builtin;
        typesystem::Type *m_type;
        llvm::Value *m_llvm_value;
        std::unique_ptr<Namespace> m_scope;
        ast::Node *m_node;
    };

}
