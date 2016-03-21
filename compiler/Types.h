//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_TYPES_H
#define JET_TYPES_H

namespace Types {

    class Type {

    public:
        virtual ~Type();

        virtual const char *name() const = 0;

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

    class Character : public Type {
    public:
        const char *name() const;
    };

    class Sequence : public Type {
    public:
        const char *name() const;
    };

    class Tuple : public Type {
    public:
        const char *name() const;
    };

    class Mapping : public Type {
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
