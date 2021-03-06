#pragma once

#include <vector>
#include <memory>

#include "../diagnostics.h"

namespace acorn::ast {

    class Node;
    class Block;
    class Name;
    class TypeName;
    class DeclName;
    class ParamName;
    class VarDecl;
    class Int;
    class Float;
    class Complex;
    class String;
    class List;
    class Dictionary;
    class Tuple;
    class Call;
    class CCall;
    class Cast;
    class Assignment;
    class Selector;
    class While;
    class If;
    class Return;
    class Spawn;
    class Case;
    class Switch;
    class Parameter;
    class Let;
    class DefDecl;
    class TypeDecl;
    class ModuleDecl;
    class Import;
    class SourceFile;

    class Visitor {
    public:
        Visitor(const char *log_name = nullptr);
        virtual ~Visitor() = default;

        virtual void visit_node(Node *node);

        template <typename T>
        void visit_node(std::unique_ptr<T> &node) {
            visit_node(node.get());
        }

    public:
        virtual void visit_block(Block *node);
        virtual void visit_name(Name *node);
        virtual void visit_type_name(TypeName *node);
        virtual void visit_decl_name(DeclName *node);
        virtual void visit_param_name(ParamName *node);
        virtual void visit_var_decl(VarDecl *node);
        virtual void visit_int(Int *node);
        virtual void visit_float(Float *node);
        virtual void visit_complex(Complex *node);
        virtual void visit_string(String *node);
        virtual void visit_list(List *node);
        virtual void visit_tuple(Tuple *node);
        virtual void visit_dictionary(Dictionary *node);
        virtual void visit_call(Call *node);
        virtual void visit_ccall(CCall *node);
        virtual void visit_cast(Cast *node);
        virtual void visit_assignment(Assignment *node);
        virtual void visit_selector(Selector *node);
        virtual void visit_while(While *node);
        virtual void visit_if(If *node);
        virtual void visit_return(Return *node);
        virtual void visit_spawn(Spawn *node);
        virtual void visit_case(Case *node);
        virtual void visit_switch(Switch *node);
        virtual void visit_parameter(Parameter *node);
        virtual void visit_let(Let *node);
        virtual void visit_def_decl(DefDecl *node);
        virtual void visit_type_decl(TypeDecl *node);
        virtual void visit_module_decl(ModuleDecl *node);
        virtual void visit_import(Import *node);
        virtual void visit_source_file(SourceFile *node);

    private:
        int m_debug_indentation;
        diagnostics::Logger m_logger;
    };

}
