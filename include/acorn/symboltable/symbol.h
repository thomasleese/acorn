#pragma once

#include <string>

#include "../diagnostics.h"

namespace llvm {
    class Value;
}

namespace acorn {

    namespace ast {
        class Node;
        class Name;
        class TypeName;
        class DeclName;
        class ParamName;
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
        Symbol(ast::TypeName *name, bool builtin);
        Symbol(ast::DeclName *name, bool builtin);
        Symbol(ast::ParamName *name, bool builtin);

        std::string name() const { return m_name; }
        void set_name(std::string name) { m_name = name; }

        bool builtin() const { return m_builtin; }
        void set_builtin(bool builtin) { m_builtin = builtin; }

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

        void copy_type_from(ast::Node *node);

        std::string to_string(int indent = 0) const;

    private:
        diagnostics::Logger m_logger;

        std::string m_name;
        bool m_builtin;
        typesystem::Type *m_type;
        llvm::Value *m_llvm_value;
        std::unique_ptr<Namespace> m_scope;
        ast::Node *m_node;
    };

}
