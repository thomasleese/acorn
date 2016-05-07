//
// Created by Thomas Leese on 06/04/2016.
//

#include "Types.h"

#include "Mangler.h"

using namespace jet;
using namespace jet::mangler;

std::string mangler::mangle_method(std::string name, types::Method *method) {
    return "_J_" + name + "_" + method->mangled_name();
}
