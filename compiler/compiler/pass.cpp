//
// Created by Thomas Leese on 07/05/2016.
//

#include <cassert>

#include "pass.h"

using namespace jet;
using namespace jet::compiler;

bool Pass::has_errors() const {
    return !m_errors.empty();
}

void Pass::push_error(errors::CompilerError *error) {
    m_errors.push_back(error);
}

errors::CompilerError *Pass::next_error() {
    assert(has_errors());

    auto error = m_errors.front();
    m_errors.erase(m_errors.begin());

    return error;
}
