//
// Created by Thomas Leese on 18/03/2016.
//

#include "Types.h"

using namespace Types;

Type::~Type() {

}

TypeType::TypeType(Type *type) {
    this->type = type;
}

const char *TypeType::name() const {
    return "TypeType";
}

const char *Any::name() const {
    return "Any";
}

const char *Void::name() const {
    return "Void";
}

const char *Boolean::name() const {
    return "Boolean";
}

const char *Integer8::name() const {
    return "Integer8";
}

const char *Integer16::name() const {
    return "Integer16";
}

const char *Integer32::name() const {
    return "Integer32";
}

const char *Integer64::name() const {
    return "Integer64";
}

const char *Integer128::name() const {
    return "Integer128";
}

const char *Float16::name() const {
    return "Float16";
}

const char *Float32::name() const {
    return "Float32";
}

const char *Float64::name() const {
    return "Float64";
}

const char *Float128::name() const {
    return "Float128";
}

const char *Character::name() const {
    return "Character";
}

const char *Sequence::name() const {
    return "Sequence";
}

const char *Tuple::name() const {
    return "Tuple";
}

const char *Mapping::name() const {
    return "Mapping";
}

const char *Function::name() const {
    return "Function";
}

const char *Record::name() const {
    return "Record";
}

const char *Union::name() const {
    return "Union";
}

