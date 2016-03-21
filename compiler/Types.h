//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_TYPES_H
#define JET_TYPES_H

namespace Types {

    class BaseType {

    };

    class Type : public BaseType {

    public:
        explicit Type(BaseType *type);

        BaseType *type;

    };

    class Any : public BaseType {

    };

    class Void : public BaseType {

    };

    class Boolean : public BaseType {

    };

    class Integer8 : public BaseType {

    };

    class Integer16 : public BaseType {

    };

    class Integer32 : public BaseType {

    };

    class Integer64 : public BaseType {

    };

    class Integer128 : public BaseType {

    };

    class Float16 : public BaseType {

    };

    class Float32 : public BaseType {

    };

    class Float64 : public BaseType {

    };

    class Float128 : public BaseType {

    };

    class Character : public BaseType {

    };

    class Sequence : public BaseType {

    };

    class Tuple : public BaseType {

    };

    class Mapping : public BaseType {

    };

    class Function : public BaseType {

    };

    class Record : public BaseType {

    };

    class Union : public BaseType {

    };

};

#endif // JET_TYPES_H
