//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_TYPES_H
#define JET_TYPES_H

#include <set>
#include <string>
#include <vector>
#include <map>

#include <llvm/IR/Constant.h>
#include <llvm/IR/Type.h>

namespace AST {
    struct Node;
    struct Argument;
}

namespace Types {

    class Type {

    public:
        virtual ~Type();

        virtual std::string name() const = 0;
        virtual bool isCompatible(const Type *other) const;

        virtual std::string mangled_name() const = 0;
        virtual llvm::Type *create_llvm_type(llvm::LLVMContext &context) const = 0;
        virtual llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const = 0;

        bool operator==(const Type &other) const;
    };

    // type constructors
    class Constructor : public Type {
    public:
        Type *create(AST::Node *node);
        virtual Type *create(AST::Node *node, std::vector<Type *> parameters) = 0;

        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;
    };

    class AnyConstructor : public Constructor {
    public:
        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);
    };

    class VoidConstructor : public Constructor {
    public:
        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);
    };

    class BooleanConstructor : public Constructor {
    public:
        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);
    };

    class IntegerConstructor : public Constructor {
    public:
        explicit IntegerConstructor(unsigned int size);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        unsigned int m_size;
    };

    class UnsignedIntegerConstructor : public Constructor {
    public:
        explicit UnsignedIntegerConstructor(unsigned int size);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        unsigned int m_size;
    };

    class FloatConstructor : public Constructor {
    public:
        explicit FloatConstructor(int size);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        int m_size;
    };

    class ArrayConstructor : public Constructor {

    public:
        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    };

    class FunctionConstructor : public Constructor {
    public:
        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);
    };

    class RecordConstructor : public Constructor {

    public:
        RecordConstructor();
        RecordConstructor(std::vector<std::string> field_names, std::vector<Type *> field_types);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        std::vector<std::string> m_field_names;
        std::vector<Type *> m_field_types;

    };

    class UnionConstructor : public Constructor {

    public:
        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    };

    class AliasConstructor : public Constructor {

    public:
        explicit AliasConstructor(AST::Node *node, Constructor *constructor, std::vector<Type *> inputParameters, std::vector<Type *> outputParameters);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        Constructor *m_constructor;
        std::map<int, int> m_parameterMapping;
        std::vector<Type *> m_knownTypes;

    };

    // concrete types
    class Parameter : public Type {
    public:
        explicit Parameter(std::string name);

        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        std::string m_name;
    };

    class Any : public Type {
    public:
        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;
    };

    class Void : public Type {
    public:
        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;
    };

    class Boolean : public Type {
    public:
        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;
    };

    class Integer : public Type {
    public:
        explicit Integer(unsigned int size);

        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        unsigned int m_size;
    };

    class UnsignedInteger : public Type {
    public:
        explicit UnsignedInteger(unsigned int size);

        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        unsigned int m_size;
    };

    class Float : public Type {
    public:
        explicit Float(int size);

        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        int m_size;
    };

    class Array : public Type {
    public:
        explicit Array(Type *element_type);

        std::string name() const;
        std::string mangled_name() const;
        Type *element_type() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        Type *m_element_type;
    };

    class Record : public Type {
    public:
        Record(std::vector<std::string> field_names, std::vector<Type *> field_types);

        bool has_field(std::string name);
        long get_field_index(std::string name);
        Type *get_field_type(std::string name);

        std::string name() const;
        std::string mangled_name() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    protected:
        std::vector<std::string> m_field_names;
        std::vector<Type *> m_field_types;
    };

    class Tuple : public Record {
    public:
        Tuple(std::vector<Type *> field_types);
    };

    class Method : public Type {
    public:
        Method(std::vector<Type *> parameter_types, Type *return_type, std::vector<std::string> official_parameter_order);
        Method(Type *return_type);
        Method(std::string parameter1_name, Type *parameter1_type, Type *return_type);
        Method(std::string parameter1_name, Type *parameter1_type, std::string parameter2_name, Type *parameter2_type, Type *return_type);

        std::string name() const;
        std::string mangled_name() const;

        Type *return_type() const;

        long get_parameter_position(std::string name) const;
        std::string get_parameter_name(long position) const;

        bool could_be_called_with(std::vector<AST::Argument *> arguments);

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        std::vector<Type *> m_parameter_types;
        Type *m_return_type;
        std::vector<std::string> m_official_parameter_order;
    };

    class Function : public Type {
    public:
        std::string name() const;
        std::string mangled_name() const;

        void add_method(Method *method);
        Method *find_method(AST::Node *node, std::vector<AST::Argument *> arguments) const;
        Method *get_method(int index) const;
        int no_methods() const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        std::vector<Method *> m_methods;
    };

    class Union : public Type {
    public:
        Union(Type *type1, Type *type2);
        Union(AST::Node *node, std::set<Type *> types);

        std::string name() const;
        std::string mangled_name() const;

        std::set<Type *> types() const;

        bool isCompatible(const Type *other) const;

        llvm::Type *create_llvm_type(llvm::LLVMContext &context) const;
        llvm::Constant *create_llvm_initialiser(llvm::LLVMContext &context) const;

    private:
        std::set<Type *> m_types;
    };

};

#endif // JET_TYPES_H
