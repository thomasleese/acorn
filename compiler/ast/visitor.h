//
// Created by Thomas Leese on 23/07/2017.
//

#pragma once

namespace acorn {

    namespace ast {

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

            virtual void visit(Block *node) = 0;
            virtual void visit(Name *node) = 0;
            virtual void visit(VariableDeclaration *node) = 0;
            virtual void visit(Int *node) = 0;
            virtual void visit(Float *node) = 0;
            virtual void visit(Complex *node) = 0;
            virtual void visit(String *node) = 0;
            virtual void visit(List *node) = 0;
            virtual void visit(Dictionary *node) = 0;
            virtual void visit(Tuple *node) = 0;
            virtual void visit(Call *node) = 0;
            virtual void visit(CCall *node) = 0;
            virtual void visit(Cast *node) = 0;
            virtual void visit(Assignment *node) = 0;
            virtual void visit(Selector *node) = 0;
            virtual void visit(While *node) = 0;
            virtual void visit(If *node) = 0;
            virtual void visit(Return *node) = 0;
            virtual void visit(Spawn *node) = 0;
            virtual void visit(Switch *node) = 0;
            virtual void visit(Parameter *node) = 0;
            virtual void visit(Let *node) = 0;
            virtual void visit(Def *node) = 0;
            virtual void visit(Type *node) = 0;
            virtual void visit(Module *node) = 0;
            virtual void visit(Import *node) = 0;
            virtual void visit(SourceFile *node) = 0;
        };

    }

}
