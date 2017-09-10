//
// Created by Thomas Leese on 23/07/2017.
//

#pragma once

namespace acorn::typesystem {

    class TypeType;
    class ParameterType;
    class VoidType;
    class BooleanType;
    class IntegerType;
    class UnsignedIntegerType;
    class FloatType;
    class UnsafePointerType;
    class FunctionType;
    class MethodType;
    class RecordType;
    class TupleType;
    class AliasType;
    class ModuleType;
    class TypeDescriptionType;
    class Type;
    class Parameter;
    class Void;
    class Boolean;
    class Integer;
    class UnsignedInteger;
    class Float;
    class UnsafePointer;
    class Record;
    class Tuple;
    class Method;
    class Function;

    class Visitor {
    public:
        virtual ~Visitor() = default;

        virtual void visit(ParameterType *type) = 0;
        virtual void visit(VoidType *type) = 0;
        virtual void visit(BooleanType *type) = 0;
        virtual void visit(IntegerType *type) = 0;
        virtual void visit(UnsignedIntegerType *type) = 0;
        virtual void visit(FloatType *type) = 0;
        virtual void visit(UnsafePointerType *type) = 0;
        virtual void visit(FunctionType *type) = 0;
        virtual void visit(MethodType *type) = 0;
        virtual void visit(RecordType *type) = 0;
        virtual void visit(TupleType *type) = 0;
        virtual void visit(AliasType *type) = 0;
        virtual void visit(ModuleType *type) = 0;
        virtual void visit(TypeDescriptionType *type) = 0;
        virtual void visit(Parameter *type) = 0;
        virtual void visit(Void *type) = 0;
        virtual void visit(Boolean *type) = 0;
        virtual void visit(Integer *type) = 0;
        virtual void visit(UnsignedInteger *type) = 0;
        virtual void visit(Float *type) = 0;
        virtual void visit(UnsafePointer *type) = 0;
        virtual void visit(Record *type) = 0;
        virtual void visit(Tuple *type) = 0;
        virtual void visit(Method *type) = 0;
        virtual void visit(Function *type) = 0;
    };

}
