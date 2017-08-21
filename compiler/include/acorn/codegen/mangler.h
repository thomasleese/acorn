//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <string>

namespace acorn::typesystem {
    class Method;
}

namespace acorn::codegen {

    std::string mangle(std::string name);
    std::string mangle_method(std::string name, typesystem::Method *type);

}
