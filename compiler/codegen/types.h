//
// Created by Thomas Leese on 27/04/2016.
//

#ifndef JET_CODGEN_TYPES_H
#define JET_CODGEN_TYPES_H

#include "../Types.h"

namespace jet {

    namespace codegen {

        class TypeGenerator : public Types::Visitor {

        };

        class TypeInitialiserGenerator : public Types::Visitor {

        };

    }

}

#endif // JET_CODGEN_TYPES_H
