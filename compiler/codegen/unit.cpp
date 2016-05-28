//
// Created by Thomas Leese on 28/05/2016.
//

#include <cassert>

#include "unit.h"

using namespace acorn;
using namespace acorn::codegen;

Unit::Unit(llvm::Value *value) :
        m_kind(Unit::Value), m_llvm_value(value)
{

}

Unit::Unit(std::string name, std::map<types::Method *, ast::FunctionDefinition *> definitions) :
        m_kind(Unit::Function), m_function_name(name), m_function_definitions(definitions)
{

}

std::string Unit::function_name() {
    assert(m_kind == Unit::Function);
    return m_function_name;
}

ast::FunctionDefinition *Unit::function_definition(types::Method *method) {
    assert(m_kind == Unit::Function);
    return m_function_definitions[method];
}

llvm::Value *Unit::llvm_value() {
    assert(m_kind == Unit::Value);
    return m_llvm_value;
}
