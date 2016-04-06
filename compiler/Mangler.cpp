//
// Created by Thomas Leese on 06/04/2016.
//

#include "Types.h"

#include "Mangler.h"

std::string Mangler::mangle_method(std::string name, Types::Method *method) {
    return "_J_" + name + "_" + method->mangled_name();
}
