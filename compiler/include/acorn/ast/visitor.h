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
    class Case;
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

        Node *visit(Node *node);

        Node *visit_node(Node *node) {
            return visit(node);
        }

    private:
        template <typename T>
        std::unique_ptr<Node> visit(std::unique_ptr<T> &node) {
            return std::unique_ptr<Node>(visit(node.release()));
        }

    public:
        virtual Node *visit_block(Block *node);
        virtual Node *visit_name(Name *node);
        virtual Node *visit_variable_declaration(VariableDeclaration *node);
        virtual Node *visit_int(Int *node);
        virtual Node *visit_float(Float *node);
        virtual Node *visit_complex(Complex *node);
        virtual Node *visit_string(String *node);
        virtual Node *visit_list(List *node);
        virtual Node *visit_tuple(Tuple *node);
        virtual Node *visit_dictionary(Dictionary *node);
        virtual Node *visit_call(Call *node);
        virtual Node *visit_ccall(CCall *node);
        virtual Node *visit_cast(Cast *node);
        virtual Node *visit_assignment(Assignment *node);
        virtual Node *visit_selector(Selector *node);
        virtual Node *visit_while(While *node);
        virtual Node *visit_if(If *node);
        virtual Node *visit_return(Return *node);
        virtual Node *visit_spawn(Spawn *node);
        virtual Node *visit_case(Case *node);
        virtual Node *visit_switch(Switch *node);
        virtual Node *visit_parameter(Parameter *node);
        virtual Node *visit_let(Let *node);
        virtual Node *visit_def(Def *node);
        virtual Node *visit_type(Type *node);
        virtual Node *visit_module(Module *node);
        virtual Node *visit_import(Import *node);
        virtual Node *visit_source_file(SourceFile *node);
    };

}
