//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef ACORN_TYPES_H
#define ACORN_TYPES_H

#include <set>
#include <string>
#include <vector>
#include <map>

namespace acorn {

    namespace ast {
        struct Node;
        struct Argument;
    }

    namespace compiler {
        class Pass;
    }

    namespace types {

        class Visitor;

        class TypeType;

        class Type {

        public:
            explicit Type();
            explicit Type(std::vector<Type *> parameters);
            virtual ~Type();

            virtual std::string name() const = 0;
            virtual std::string mangled_name() const = 0;

            virtual bool is_compatible(const Type *other) const;

            virtual TypeType *type() const = 0;

            virtual Type *with_parameters(std::vector<Type *> parameters) = 0;

            std::vector<Type *> parameters() const;

            virtual void accept(Visitor *visitor) = 0;

        protected:
            std::vector<Type *> m_parameters;
        };

        class Parameter;

        // type "type"s -- i.e. the type of concrete types
        class TypeType : public Type {
        public:
            explicit TypeType();
            explicit TypeType(std::vector<TypeType *> parameters);
            virtual Type *create(compiler::Pass *pass, ast::Node *node) = 0;

            std::string mangled_name() const;

            virtual TypeType *type() const;

            TypeType *with_parameters(std::vector<Type *> parameters);
            virtual TypeType *with_parameters(std::vector<TypeType *> parameters) = 0;
        };

        class ParameterType : public TypeType {
        public:
            std::string name() const;

            bool is_compatible(const Type *other) const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            ParameterType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class AnyType : public TypeType {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            AnyType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class VoidType : public TypeType {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            VoidType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class BooleanType : public TypeType {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            BooleanType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class IntegerType : public TypeType {
        public:
            IntegerType(unsigned int size);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            IntegerType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsignedIntegerType : public TypeType {
        public:
            UnsignedIntegerType(unsigned int size);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            UnsignedIntegerType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class FloatType : public TypeType {
        public:
            FloatType(int size);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            FloatType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        private:
            int m_size;
        };

        class UnsafePointerType : public TypeType {
        public:
            explicit UnsafePointerType(TypeType *element_type = nullptr);

            std::string name() const;

            bool has_element_type() const;
            TypeType *element_type() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            UnsafePointerType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class FunctionType : public TypeType {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            FunctionType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class RecordType : public TypeType {

        public:
            RecordType();
            RecordType(std::vector<ParameterType *> input_parameters,
                       std::vector<std::string> field_names,
                       std::vector<TypeType *> field_types);
            RecordType(std::vector<ParameterType *> input_parameters,
                       std::vector<std::string> field_names,
                       std::vector<TypeType *> field_types,
                       std::vector<TypeType *> parameters);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            RecordType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        private:
            std::vector<ParameterType *> m_input_parameters;
            std::vector<std::string> m_field_names;
            std::vector<TypeType *> m_field_types;

        };

        class UnionType : public TypeType {

        public:
            UnionType();
            explicit UnionType(std::vector<TypeType *> parameters);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            UnionType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        };

        class TupleType : public TypeType {
        public:
            TupleType();
            explicit TupleType(std::vector<TypeType *> parameters);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            TupleType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        class AliasType : public TypeType {

        public:
            AliasType(TypeType *alias, std::vector<ParameterType *> input_parameters);
            AliasType(TypeType *alias, std::vector<ParameterType *> input_parameters, std::vector<TypeType *> parameters);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            AliasType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        private:
            TypeType *m_alias;
            std::vector<ParameterType *> m_input_parameters;
            std::map<int, int> m_parameterMapping;
            std::vector<Type *> m_knownTypes;

        };

        class Method; // FIXME replace with 'MethodType'

        class ProtocolType : public TypeType {
        public:
            explicit ProtocolType(std::vector<ParameterType *> input_parameters, std::vector<Method *> methods);
            ProtocolType(std::vector<ParameterType *> input_parameters, std::vector<Method *> methods, std::vector<TypeType *> parameters);

            std::string name() const;

            bool is_compatible(const Type *other) const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            ProtocolType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);

        private:
            std::vector<ParameterType *> m_input_parameters;
            std::vector<Method *> m_methods;
        };

        class TypeDescriptionType : public TypeType {
        public:
            explicit TypeDescriptionType(TypeType *type = nullptr);

            std::string name() const;

            bool is_compatible(const Type *other) const;

            Type *create(compiler::Pass *pass, ast::Node *node);

            TypeDescriptionType *with_parameters(std::vector<TypeType *> parameters);

            void accept(Visitor *visitor);
        };

        // constructed types
        class Parameter : public Type {

        public:
            Parameter(ParameterType *constructor);

            std::string name() const;
            std::string mangled_name() const;

            ParameterType *type() const;

            bool is_compatible(const Type *other) const;

            Parameter *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            ParameterType *m_constructor;

        };

        class Any : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            AnyType *type() const;

            Any *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class Void : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            VoidType *type() const;

            Void *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class Boolean : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            BooleanType *type() const;

            Boolean *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class Integer : public Type {
        public:
            explicit Integer(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            IntegerType *type() const;

            unsigned int size() const;

            Integer *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsignedInteger : public Type {
        public:
            explicit UnsignedInteger(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            UnsignedIntegerType *type() const;

            unsigned int size() const;

            UnsignedInteger *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class Float : public Type {
        public:
            explicit Float(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            FloatType *type() const;

            unsigned int size() const;

            Float *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsafePointer : public Type {
        public:
            explicit UnsafePointer(Type *element_type);

            std::string name() const;
            std::string mangled_name() const;

            UnsafePointerType *type() const;

            Type *element_type() const;

            bool is_compatible(const Type *other) const;

            UnsafePointer *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class Record : public Type {
        public:
            Record(std::vector<std::string> field_names, std::vector<Type *> field_types);

            bool has_field(std::string name);
            long get_field_index(std::string name);
            Type *get_field_type(std::string name);
            std::vector<Type *> field_types() const;

            std::string name() const;
            std::string mangled_name() const;

            RecordType *type() const;

            bool is_compatible(const Type *other) const;

            Record *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        protected:
            std::vector<std::string> m_field_names;
        };

        class Tuple : public Record {
        public:
            Tuple(std::vector<Type *> field_types);

            void accept(Visitor *visitor);
        };

        class Method : public Type {
        public:
            Method(std::vector<Type *> parameter_types, Type *return_type);
            Method(Type *return_type);
            Method(Type *parameter1_type, Type *return_type);
            Method(Type *parameter1_type, Type *parameter2_type, Type *return_type);
            Method(Type *parameter1_type, Type *parameter2_type, Type *parameter3_type, Type *return_type);

            std::string name() const;
            std::string mangled_name() const;

            TypeType *type() const;

            void set_is_generic(bool is_generic);
            bool is_generic() const;

            std::vector<Type *> parameter_types() const;
            Type *return_type() const;
            bool could_be_called_with(std::vector<Type *> arguments);

            void set_parameter_inout(Type *type, bool inout);
            bool is_parameter_inout(Type *type);

            Method *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            std::map<Type *, bool> m_inouts;
            bool m_is_generic;
        };

        class Function : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            FunctionType *type() const;

            void add_method(Method *method);
            Method *find_method(ast::Node *node, std::vector<Type *> arguments) const;
            Method *get_method(int index) const;
            int no_methods() const;

            Function *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class Union : public Type {
        public:
            Union(Type *type1, Type *type2);
            Union(std::vector<Type *> types);

            std::string name() const;
            std::string mangled_name() const;

            UnionType *type() const;

            std::vector<Type *> types() const;
            uint8_t type_index(const Type *type, bool *exists) const;

            bool is_compatible(const Type *other) const;

            Union *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class Protocol : public Type {
        public:
            explicit Protocol(ProtocolType *type);
            Protocol(std::vector<Method *> methods, ProtocolType *type);

            std::string name() const;
            std::string mangled_name() const;

            ProtocolType *type() const;
            std::vector<Method *> methods() const;

            bool is_compatible(const Type *other) const;

            Protocol *with_parameters(std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            ProtocolType *m_type;
        };

        class Visitor {
        public:
            virtual ~Visitor();

            virtual void visit(ParameterType *type) = 0;
            virtual void visit(AnyType *type) = 0;
            virtual void visit(VoidType *type) = 0;
            virtual void visit(BooleanType *type) = 0;
            virtual void visit(IntegerType *type) = 0;
            virtual void visit(UnsignedIntegerType *type) = 0;
            virtual void visit(FloatType *type) = 0;
            virtual void visit(UnsafePointerType *type) = 0;
            virtual void visit(FunctionType *type) = 0;
            virtual void visit(RecordType *type) = 0;
            virtual void visit(UnionType *type) = 0;
            virtual void visit(TupleType *type) = 0;
            virtual void visit(AliasType *type) = 0;
            virtual void visit(ProtocolType *type) = 0;
            virtual void visit(TypeDescriptionType *type) = 0;

            virtual void visit(Parameter *type) = 0;
            virtual void visit(Any *type) = 0;
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
            virtual void visit(Union *type) = 0;
            virtual void visit(Protocol *type) = 0;
        };

    };

}

#endif // ACORN_TYPES_H
