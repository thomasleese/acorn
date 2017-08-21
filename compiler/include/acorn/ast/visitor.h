//
// Created by Thomas Leese on 23/07/2017.
//

#pragma once

#include <vector>

namespace acorn::ast {

    class Node;
    class Block;
    class Name;
    class VariableDeclaration;
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
    class Switch;
    class Parameter;
    class Let;
    class Def;
    class Type;
    class Module;
    class Import;
    class SourceFile;

    class Visitor {
    public:
        virtual ~Visitor();

        template <typename T>
        void accept(const std::unique_ptr<T> &node) {
            node->accept(this);
        }

        void accept(Node *node);
        void accept_if_present(Node *node);

        template <typename T>
        void accept_many(const std::vector<std::unique_ptr<T>> &nodes) {
            for (auto &node : nodes) {
                node->accept(this);
            }
        }

        virtual void visit_block(Block *node) = 0;
        virtual void visit_name(Name *node) = 0;
        virtual void visit_variable_declaration(VariableDeclaration *node) = 0;
        virtual void visit_int(Int *node) = 0;
        virtual void visit_float(Float *node) = 0;
        virtual void visit_complex(Complex *node) = 0;
        virtual void visit_string(String *node) = 0;
        virtual void visit_list(List *node) = 0;
        virtual void visit_dictionary(Dictionary *node) = 0;
        virtual void visit_tuple(Tuple *node) = 0;
        virtual void visit_call(Call *node) = 0;
        virtual void visit_ccall(CCall *node) = 0;
        virtual void visit_cast(Cast *node) = 0;
        virtual void visit_assignment(Assignment *node) = 0;
        virtual void visit_selector(Selector *node) = 0;
        virtual void visit_while(While *node) = 0;
        virtual void visit_if(If *node) = 0;
        virtual void visit_return(Return *node) = 0;
        virtual void visit_spawn(Spawn *node) = 0;
        virtual void visit_switch(Switch *node) = 0;
        virtual void visit_parameter(Parameter *node) = 0;
        virtual void visit_let(Let *node) = 0;
        virtual void visit_def(Def *node) = 0;
        virtual void visit_type(Type *node) = 0;
        virtual void visit_module(Module *node) = 0;
        virtual void visit_import(Import *node) = 0;
        virtual void visit_source_file(SourceFile *node) = 0;
    };

}
