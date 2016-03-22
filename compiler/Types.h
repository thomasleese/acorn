//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_TYPES_H
#define JET_TYPES_H

#include <string>
#include <map>

namespace Types {

    class Type {

    public:
        virtual ~Type();

        virtual const char *name() const = 0;

        bool operator==(const Type &other) const;
    };

    class Parameter : public Type {
    public:
        Parameter(std::string name);

        const char *name() const;

    private:
        std::string m_name;
    };

    class TypeType : public Type {
    public:
        explicit TypeType(Type *type);

        const char *name() const;

        Type *type;
    };

    class Any : public Type {
    public:
        const char *name() const;
    };

    class Void : public Type {
    public:
        const char *name() const;
    };

    class Boolean : public Type {
    public:
        const char *name() const;
    };

    class Integer8 : public Type {
    public:
        const char *name() const;
    };

    class Integer16 : public Type {
    public:
        const char *name() const;
    };

    class Integer32 : public Type {
    public:
        const char *name() const;
    };

    class Integer64 : public Type {
    public:
        const char *name() const;
    };

    class Integer128 : public Type {
    public:
        const char *name() const;
    };

    class Float16 : public Type {
    public:
        const char *name() const;
    };

    class Float32 : public Type {
    public:
        const char *name() const;
    };

    class Float64 : public Type {
    public:
        const char *name() const;
    };

    class Float128 : public Type {
    public:
        const char *name() const;
    };

    class Sequence : public Type {
    public:
        explicit Sequence(Type *elementType);

        const char *name() const;

    private:
        Type *m_elementType;
    };

    class Product : public Type {
    public:
        const char *name() const;
    };

    class Function : public Type {
    public:
        const char *name() const;
    };

    class Record : public Type {
    public:
        const char *name() const;
    };

    class Union : public Type {
    public:
        const char *name() const;
    };

};

#endif // JET_TYPES_H
