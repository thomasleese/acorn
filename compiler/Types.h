//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_TYPES_H
#define JET_TYPES_H

#include <set>
#include <string>
#include <vector>
#include <map>

namespace AST {
    struct Node;
}

namespace Types {

    class Type {

    public:
        virtual ~Type();

        virtual std::string name() const = 0;
        virtual bool isCompatible(const Type *other) const;

        bool operator==(const Type &other) const;
    };

    // type constructors
    class Constructor : public Type {
    public:
        virtual Type *create(AST::Node *node, std::vector<Type *> parameters) = 0;
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
        explicit IntegerConstructor(int size);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        int m_size;
    };

    class FloatConstructor : public Constructor {
    public:
        explicit FloatConstructor(int size);

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

    private:
        int m_size;
    };

    class SequenceConstructor : public Constructor {

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
        explicit RecordConstructor();

        std::string name() const;

        Type *create(AST::Node *node, std::vector<Type *> parameters);

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

    private:
        std::string m_name;
    };

    class Any : public Type {
    public:
        std::string name() const;
    };

    class Void : public Type {
    public:
        std::string name() const;
    };

    class Boolean : public Type {
    public:
        std::string name() const;
    };

    class Integer : public Type {
    public:
        explicit Integer(int size);

        std::string name() const;

    private:
        int m_size;
    };

    class Float : public Type {
    public:
        explicit Float(int size);

        std::string name() const;

    private:
        int m_size;
    };

    class Sequence : public Type {
    public:
        explicit Sequence(Type *elementType);

        std::string name() const;

    private:
        Type *m_elementType;
    };

    class Product : public Type {
    public:
        std::string name() const;
    };

    class Method : public Type {
    public:
        explicit Method(std::map<std::string, Type *> parameter_types, Type *return_type);

        std::string name() const;
        Type *return_type() const;

        bool could_be_called_with(std::map<std::string, Type *> parameters);

    private:
        std::map<std::string, Type *> m_parameter_types;
        Type *m_return_type;
    };

    class Function : public Type {
    public:
        std::string name() const;

        void add_method(Method *method);
        Method *find_method(AST::Node *node, std::map<std::string, Type *> parameters) const;

    private:
        std::vector<Method *> m_methods;
    };

    class Record : public Type {
    public:
        std::string name() const;
    };

    class Union : public Type {
    public:
        explicit Union(AST::Node *node, std::set<Type *> types);

        std::string name() const;
        std::set<Type *> types() const;

        bool isCompatible(const Type *other) const;

    private:
        std::set<Type *> m_types;
    };

};

#endif // JET_TYPES_H
