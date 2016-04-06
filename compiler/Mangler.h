//
// Created by Thomas Leese on 06/04/2016.
//

#ifndef JET_MANGLER_H
#define JET_MANGLER_H

#include <string>

namespace Types {
    class Method;
}

namespace Mangler {

    std::string mangle_method(std::string name, Types::Method *method);

};

#endif //JET_MANGLER_H
