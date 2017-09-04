//
// Created by Thomas Leese on 12/01/2017.
//

#include "acorn/typesystem/types.h"

#include "acorn/codegen/mangler.h"

using namespace acorn;

std::string codegen::mangle(std::string name) {
    return "_A_" + name;
}

std::string codegen::mangle_method(std::string name, typesystem::Method *type) {
    return mangle(name + "_" + type->mangled_name());
}
