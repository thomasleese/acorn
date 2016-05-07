//
// Created by Thomas Leese on 06/04/2016.
//

#ifndef JET_MANGLER_H
#define JET_MANGLER_H

#include <string>

namespace jet {

    namespace types {
        class Method;
    }

    namespace mangler {

        std::string mangle_method(std::string name, types::Method *method);

    }

}

#endif //JET_MANGLER_H
