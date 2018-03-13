#pragma once

#include <set>
#include <string>
#include <vector>
#include <map>

namespace acorn {

    namespace ast {
        class Node;
        class Node;
        class Call;
    }

    namespace diagnostics {
        class Reporter;
    }

}

namespace acorn::typesystem {

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

    // type "type"s -- i.e. the type of concrete typesystem
    class TypeType : public Type {
    public:
        explicit TypeType();
        explicit TypeType(std::vector<TypeType *> parameters);
        virtual Type *create(diagnostics::Reporter *diagnostics, ast::Node *node) = 0;

        std::string mangled_name() const;

        virtual TypeType *type() const;

        TypeType *with_parameters(std::vector<Type *> parameters);
        virtual TypeType *with_parameters(std::vector<TypeType *> parameters) = 0;
    };

    class ParameterType : public TypeType {
    public:
        std::string name() const;

        bool is_compatible(const Type *other) const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        ParameterType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class VoidType : public TypeType {
    public:
        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        VoidType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class BooleanType : public TypeType {
    public:
        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        BooleanType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class IntegerType : public TypeType {
    public:
        IntegerType(unsigned int size);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        IntegerType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        unsigned int m_size;
    };

    class UnsignedIntegerType : public TypeType {
    public:
        UnsignedIntegerType(unsigned int size);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        UnsignedIntegerType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        unsigned int m_size;
    };

    class FloatType : public TypeType {
    public:
        FloatType(int size);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

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

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        UnsafePointerType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class FunctionType : public TypeType {
    public:
        FunctionType();
        FunctionType(std::vector<TypeType *> parameters);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        FunctionType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class MethodType : public TypeType {
    public:
        MethodType();
        MethodType(std::vector<TypeType *> parameters);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        MethodType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class Function;

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

        Function *constructor() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        RecordType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        void create_builtin_constructor();

        std::vector<ParameterType *> m_input_parameters;
        std::vector<std::string> m_field_names;
        std::vector<TypeType *> m_field_types;
        Function *m_constructor;

    };

    class TupleType : public TypeType {
    public:
        TupleType();
        explicit TupleType(std::vector<TypeType *> parameters);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        TupleType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class AliasType : public TypeType {

    public:
        AliasType(TypeType *alias, std::vector<ParameterType *> input_parameters);
        AliasType(TypeType *alias, std::vector<ParameterType *> input_parameters, std::vector<TypeType *> parameters);

        std::string name() const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        AliasType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        TypeType *m_alias;
        std::vector<ParameterType *> m_input_parameters;
        std::map<int, int> m_parameterMapping;
        std::vector<Type *> m_knownTypes;

    };

    class ModuleType : public Type {
    public:
        std::string name() const;
        std::string mangled_name() const;

        TypeType *type() const;
        ModuleType *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);
    };

    class TypeDescriptionType : public TypeType {
    public:
        explicit TypeDescriptionType(TypeType *type = nullptr);

        std::string name() const;

        bool is_compatible(const Type *other) const;

        Type *create(diagnostics::Reporter *diagnostics, ast::Node *node);

        TypeDescriptionType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    // constructed typesystem
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

        bool has_child(std::string name);
        Type *child_type(std::string name);

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
        int parameter_index(std::string name) const;
        Type *return_type() const;

        template<typename T> std::vector<T> ordered_arguments(std::vector<T> positional_arguments, std::map<std::string, T> keyword_arguments, bool *valid = nullptr);
        std::vector<ast::Node *> ordered_arguments(ast::Call *call, bool *valid = nullptr);
        std::vector<Type *> ordered_argument_types(ast::Call *call, bool *valid = nullptr);

        bool could_be_called_with(std::vector<Type *> positional_arguments, std::map<std::string, Type *> keyword_arguments);

        void add_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation);
        std::vector<std::map<typesystem::ParameterType *, typesystem::Type *> > generic_specialisations();
        size_t no_generic_specialisation() const;
        void add_empty_specialisation();

        void set_parameter_inout(Type *type, bool inout);
        bool is_parameter_inout(Type *type);

        void set_parameter_name(int index, std::string name);

        Method *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        std::map<Type *, bool> m_inouts;
        std::map<std::string, int> m_names;
        bool m_is_generic;
        std::vector<std::map<typesystem::ParameterType *, typesystem::Type *> > m_specialisations;
    };

    class Function : public Type {
    public:
        std::string name() const;
        std::string mangled_name() const;

        FunctionType *type() const;

        void add_method(Method *method);
        Method *find_method(ast::Node *node, std::vector<Type *> positional_arguments, std::map<std::string, Type *> keyword_arguments) const;
        Method *find_method(ast::Call *call) const;
        Method *get_method(int index) const;
        int no_methods() const;
        std::vector<Method *> methods() const;
        int index_of(Method *method) const;

        void set_llvm_index(Method *method, int index);
        int get_llvm_index(Method *method);

        Function *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        std::map<Method *, int> m_llvm_index;
    };

}
