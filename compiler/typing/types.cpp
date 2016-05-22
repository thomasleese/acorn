//
// Created by Thomas Leese on 18/03/2016.
//

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>

#include "../ast/nodes.h"
#include "../compiler/pass.h"
#include "../errors.h"

#include "types.h"

using namespace acorn;
using namespace acorn::types;

Type::~Type() {

}

bool Type::is_compatible(const Type *other) const {
    auto name1 = name();
    auto name2 = other->name();
    return name1 == name2;
}

Type *Type::get_parameter(int i) const {
    assert(i < m_parameters.size());
    return m_parameters[i];
}

void Type::set_parameter(int i, Type *type) {
    assert(i < m_parameters.size());
    m_parameters[i] = type;
}

std::vector<types::Type *> Type::parameters() const {
    return m_parameters;
}

Type *TypeType::create(compiler::Pass *pass, ast::Node *node) {
    return create(pass, node, std::vector<Type *>());
}

std::string TypeType::mangled_name() const {
    return "?";
}

TypeType *TypeType::type() const {
    return new TypeDescriptionType();
}

std::string ParameterType::name() const {
    return "ParameterType";
}

bool ParameterType::is_compatible(const Type *other) const {
    return (bool) dynamic_cast<const types::TypeType *>(other);
}

Type *ParameterType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Parameter(this);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

ParameterType *ParameterType::clone() const {
    return new ParameterType();
}

void ParameterType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string AnyType::name() const {
    return "AnyType";
}

Type *AnyType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Any();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

AnyType *AnyType::clone() const {
    return new AnyType();
}

void AnyType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string VoidType::name() const {
    return "VoidType";
}

Type *VoidType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Void();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

VoidType *VoidType::clone() const {
    return new VoidType();
}

void VoidType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string BooleanType::name() const {
    return "BooleanType";
}

Type *BooleanType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Boolean();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

BooleanType *BooleanType::clone() const {
    return new BooleanType();
}

void BooleanType::accept(Visitor *visitor) {
    visitor->visit(this);
}

IntegerType::IntegerType(unsigned int size) : m_size(size) {

}

std::string IntegerType::name() const {
    std::stringstream ss;
    ss << "IntegerType" << m_size;
    return ss.str();
}

Type *IntegerType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Integer(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

IntegerType *IntegerType::clone() const {
    return new IntegerType(m_size);
}

void IntegerType::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnsignedIntegerType::UnsignedIntegerType(unsigned int size) : m_size(size) {

}

std::string UnsignedIntegerType::name() const {
    std::stringstream ss;
    ss << "UnsignedIntegerType" << m_size;
    return ss.str();
}

Type *UnsignedIntegerType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new UnsignedInteger(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

UnsignedIntegerType *UnsignedIntegerType::clone() const {
    return new UnsignedIntegerType(m_size);
}

void UnsignedIntegerType::accept(Visitor *visitor) {
    visitor->visit(this);
}

FloatType::FloatType(int size) : m_size(size) {

}

std::string FloatType::name() const {
    std::stringstream ss;
    ss << "FloatType" << m_size;
    return ss.str();
}

Type *FloatType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Float(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

FloatType *FloatType::clone() const {
    return new FloatType(m_size);
}

void FloatType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string UnsafePointerType::name() const {
    return "UnsafePointerType";
}

Type *UnsafePointerType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() == 1 && parameters[0] != nullptr) {
        return new UnsafePointer(parameters[0]);
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    }
}

UnsafePointerType *UnsafePointerType::clone() const {
    return new UnsafePointerType();
}

void UnsafePointerType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string FunctionType::name() const {
    return "FunctionType";
}

Type *FunctionType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    Function *function = new Function();

    for (auto parameter : parameters) {
        Method *method = dynamic_cast<Method *>(parameter);
        if (method == nullptr) {
            pass->push_error(new errors::InvalidTypeParameters(node, 0, 0));
            continue;
        }

        function->add_method(method);
    }

    return function;
}

FunctionType *FunctionType::clone() const {
    return new FunctionType();
}

void FunctionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

RecordType::RecordType() {

}

RecordType::RecordType(std::vector<Parameter *> input_parameters, std::vector<std::string> field_names, std::vector<TypeType *> field_types, std::vector<std::vector<Type *> > field_parameters) :
        m_input_parameters(input_parameters), m_field_names(field_names), m_field_types(field_types), m_field_parameters(field_parameters) {

}

std::string RecordType::name() const {
    std::stringstream ss;
    ss << "RecordType{";
    for (auto type : m_field_types) {
        ss << type->name() << ",";
    }
    ss << "}";
    return ss.str();
}

Type *RecordType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() == m_input_parameters.size()) {
        std::map<Parameter *, Type *> parameter_mapping;
        for (int i = 0; i < m_input_parameters.size(); i++) {
            parameter_mapping[m_input_parameters[i]] = parameters[i];
        }

        std::vector<Type *> field_types;

        for (int i = 0; i < m_field_types.size(); i++) {
            std::vector<Type *> field_parameters;
            for (auto t : m_field_parameters[i]) {
                auto type = t;
                if (auto p = dynamic_cast<Parameter *>(t)) {
                    type = parameter_mapping[p];
                }

                field_parameters.push_back(type);
            }

            auto result = m_field_types[i]->create(pass, node, field_parameters);
            if (result == nullptr) {
                return nullptr;
            }
            field_types.push_back(result);
        }

        return new Record(m_field_names, field_types);
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), m_input_parameters.size()));
        return nullptr;
    }
}

RecordType *RecordType::clone() const {
    std::vector<Parameter *> input_parameters;
    for (auto element : m_input_parameters) {
        input_parameters.push_back(element->clone());
    }

    std::vector<TypeType *> field_types;
    for (auto element : m_field_types) {
        field_types.push_back(element->clone());
    }

    std::vector<std::vector<Type *> > field_parameters;
    for (auto p : m_field_parameters) {
        std::vector<Type *> v;
        for (auto a : p) {
            v.push_back(a->clone());
        }
        field_parameters.push_back(v);
    }

    return new RecordType(input_parameters, m_field_names, field_types, field_parameters);
}

void RecordType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string UnionType::name() const {
    return "UnionType";
}

Type *UnionType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    } else {
        return new Union(parameters);
    }
}

UnionType *UnionType::clone() const {
    return new UnionType();
}

void UnionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string TupleType::name() const {
    return "TupleType";
}

Type *TupleType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    } else {
        return new Tuple(parameters);
    }
}

TupleType *TupleType::clone() const {
    return new TupleType();
}

void TupleType::accept(Visitor *visitor) {
    visitor->visit(this);
}

AliasType::AliasType(TypeType *constructor, std::vector<Parameter *> input_parameters, std::vector<Type *> output_parameters) :
        m_constructor(constructor),
        m_input_parameters(input_parameters),
        m_output_parameters(output_parameters)
{

}

std::string AliasType::name() const {
    return m_constructor->name();
}

Type *AliasType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    for (unsigned long i = 0; i < m_input_parameters.size(); i++) {
        auto parameter = m_input_parameters[i];

        int mapping = -1;

        for (unsigned long j = 0; j < m_output_parameters.size(); j++) {
            if (parameter->is_compatible(m_output_parameters[j])) {
                mapping = j;
                break;
            }
        }

        if (mapping == -1) {
            pass->push_error(new errors::InvalidTypeParameters(node, 0, 0));
            return nullptr;
        }

        m_parameterMapping[i] = mapping;
    }

    for (auto t : m_output_parameters) {
        if (dynamic_cast<Parameter *>(t) == nullptr) {
            m_knownTypes.push_back(t);
        }
    }

    if (parameters.size() != m_parameterMapping.size()) {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), m_parameterMapping.size()));
        return nullptr;
    }

    std::vector<Type *> actualParameters;
    for (unsigned long i = 0; i < parameters.size() + m_knownTypes.size(); i++) {
        actualParameters.push_back(nullptr);
    }

    for (unsigned long i = 0; i < parameters.size(); i++) {
        if (m_parameterMapping.find(i) != m_parameterMapping.end()) {
            actualParameters[m_parameterMapping[i]] = parameters[i];
        }
    }

    unsigned long j = 0;
    for (unsigned long i = 0; i < actualParameters.size(); i++) {
        if (actualParameters[i] == nullptr) {
            actualParameters[i] = m_knownTypes[j];
            j++;
        }
    }

    return m_constructor->create(pass, node, actualParameters);
}

AliasType *AliasType::clone() const {
    std::vector<Parameter *> input_parameters;
    for (auto p : m_input_parameters) {
        input_parameters.push_back(p->clone());
    }

    std::vector<Type *> output_parameters;
    for (auto p : m_output_parameters) {
        output_parameters.push_back(p->clone());
    }

    return new AliasType(m_constructor->clone(), input_parameters, output_parameters);
}

void AliasType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string TypeDescriptionType::name() const {
    return "TypeDescriptionType";
}

Type *TypeDescriptionType::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() == 1) {
        return parameters[0]->type();
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    }
}

TypeDescriptionType *TypeDescriptionType::clone() const {
    return new TypeDescriptionType();
}

void TypeDescriptionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(ParameterType *constructor) : m_constructor(constructor) {

}

std::string Parameter::name() const {
    return "Parameter{Any}";
}

std::string Parameter::mangled_name() const {
    return "p";
}

ParameterType *Parameter::type() const {
    return m_constructor;
}

bool Parameter::is_compatible(const Type *other) const {
    return true; // acts like Any
}

Parameter *Parameter::clone() const {
    // don't clone type
    return new Parameter(m_constructor);
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Any::name() const {
    return "Any";
}

std::string Any::mangled_name() const {
    return "a";
}

AnyType *Any::type() const {
    return new AnyType();
}

Any *Any::clone() const {
    return new Any();
}

void Any::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Void::name() const {
    return "Void";
}

std::string Void::mangled_name() const {
    return "v";
}

VoidType *Void::type() const {
    return new VoidType();
}

Void *Void::clone() const {
    return new Void();
}

void Void::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Boolean::name() const {
    return "Boolean";
}

std::string Boolean::mangled_name() const {
    return "b";
}

BooleanType *Boolean::type() const {
    return new BooleanType();
}

Boolean *Boolean::clone() const {
    return new Boolean();
}

void Boolean::accept(Visitor *visitor) {
    visitor->visit(this);
}

Integer::Integer(unsigned int size) : m_size(size) {

}

std::string Integer::name() const {
    std::stringstream ss;
    ss << "Integer" << m_size;
    return ss.str();
}

std::string Integer::mangled_name() const {
    std::stringstream ss;
    ss << "i" << m_size;
    return ss.str();
}

IntegerType *Integer::type() const {
    return new IntegerType(m_size);
}

unsigned int Integer::size() const {
    return m_size;
}

Integer *Integer::clone() const {
    return new Integer(m_size);
}

void Integer::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnsignedInteger::UnsignedInteger(unsigned int size) : m_size(size) {

}

std::string UnsignedInteger::name() const {
    std::stringstream ss;
    ss << "UnsignedInteger" << m_size;
    return ss.str();
}

std::string UnsignedInteger::mangled_name() const {
    std::stringstream ss;
    ss << "ui" << m_size;
    return ss.str();
}

UnsignedIntegerType *UnsignedInteger::type() const {
    return new UnsignedIntegerType(m_size);
}

unsigned int UnsignedInteger::size() const {
    return m_size;
}

UnsignedInteger *UnsignedInteger::clone() const {
    return new UnsignedInteger(m_size);
}

void UnsignedInteger::accept(Visitor *visitor) {
    visitor->visit(this);
}

Float::Float(unsigned int size) : m_size(size) {

}

std::string Float::name() const {
    std::stringstream ss;
    ss << "Float" << m_size;
    return ss.str();
}

std::string Float::mangled_name() const {
    std::stringstream ss;
    ss << "f" << m_size;
    return ss.str();
}

FloatType *Float::type() const {
    return new FloatType(m_size);
}

unsigned int Float::size() const {
    return m_size;
}

Float *Float::clone() const {
    return new Float(m_size);
}

void Float::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnsafePointer::UnsafePointer(Type *element_type) {
    m_parameters.push_back(element_type);
}

std::string UnsafePointer::name() const {
    std::stringstream ss;
    ss << "UnsafePointer{" << element_type()->name() << "}";
    return ss.str();
}

std::string UnsafePointer::mangled_name() const {
    std::stringstream ss;
    ss << "p" << element_type()->mangled_name();
    return ss.str();
}

UnsafePointerType *UnsafePointer::type() const {
    return new UnsafePointerType();
}

Type *UnsafePointer::element_type() const {
    assert(m_parameters.size() == 1);
    return m_parameters[0];
}

bool UnsafePointer::is_compatible(const Type *other) const {
    auto other_pointer = dynamic_cast<const UnsafePointer *>(other);
    if (other_pointer) {
        return element_type()->is_compatible(other_pointer->element_type());
    } else {
        return false;
    }
}

UnsafePointer *UnsafePointer::clone() const {
    return new UnsafePointer(element_type()->clone());
}

void UnsafePointer::accept(Visitor *visitor) {
    visitor->visit(this);
}

Record::Record(std::vector<std::string> field_names, std::vector<Type *> field_types) :
        m_field_names(field_names)
{
    m_parameters = field_types;
}

bool Record::has_field(std::string name) {
    for (auto &field_name : m_field_names) {
        if (field_name == name) {
            return true;
        }
    }

    return false;
}

long Record::get_field_index(std::string name) {
    for (long i = 0; i < m_field_names.size(); i++) {
        if (m_field_names[i] == name) {
            return i;
        }
    }

    return -1;
}

Type *Record::get_field_type(std::string name) {
    long index = get_field_index(name);
    if (index < 0) {
        return nullptr;
    }

    return m_parameters[index];
}

std::vector<Type *> Record::field_types() const {
    return m_parameters;
}

std::string Record::name() const {
    std::stringstream ss;
    ss << "Record{";
    for (auto type : m_parameters) {
        ss << type->name();
        if (type != m_parameters.back()) {
            ss << ", ";
        }
    }
    ss << "}";
    return ss.str();
}

std::string Record::mangled_name() const {
    std::stringstream ss;
    ss << "r";
    for (auto type : m_parameters) {
        ss << type->mangled_name();
    }
    return ss.str();
}

RecordType *Record::type() const {
    return new RecordType();
}

bool Record::is_compatible(const Type *other) const {
    auto other_record = dynamic_cast<const Record *>(other);
    if (other_record) {
        if (m_parameters.size() == other_record->m_parameters.size()) {
            for (int i = 0; i < m_parameters.size(); i++) {
                if (!m_parameters[i]->is_compatible(other_record->m_parameters[i])) {
                    return false;
                }
            }

            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

Record *Record::clone() const {
    std::vector<Type *> field_types;
    for (auto t : m_parameters) {
        field_types.push_back(t->clone());
    }

    return new Record(m_field_names, field_types);
}

void Record::accept(Visitor *visitor) {
    visitor->visit(this);
}

Tuple::Tuple(std::vector<Type *> field_types) : Record(std::vector<std::string>(), field_types) {
    for (int i = 0; i < field_types.size(); i++) {
        std::stringstream ss;
        ss << i;
        m_field_names.push_back(ss.str());
    }
}

void Tuple::accept(Visitor *visitor) {
    visitor->visit(this);
}

Method::Method(std::vector<Type *> parameter_types, Type *return_type) {
    m_parameters.push_back(return_type);
    for (auto p : parameter_types) {
        m_parameters.push_back(p);
    }
}

Method::Method(Type *return_type) : m_is_generic(false) {
    m_parameters.push_back(return_type);
}

Method::Method(Type *parameter1_type, Type *return_type) : Method(return_type) {
    m_parameters.push_back(parameter1_type);
}

Method::Method(Type *parameter1_type, Type *parameter2_type, Type *return_type) : Method(return_type) {
    m_parameters.push_back(parameter1_type);
    m_parameters.push_back(parameter2_type);
}

Method::Method(Type *parameter1_type, Type *parameter2_type, Type *parameter3_type, Type *return_type) : Method(return_type) {
    m_parameters.push_back(parameter1_type);
    m_parameters.push_back(parameter2_type);
    m_parameters.push_back(parameter3_type);
}

std::string Method::name() const {
    std::stringstream ss;
    ss << "Method{";
    for (auto type : m_parameters) {
        ss << type->name() << ", ";
    }
    ss << "}";
    return ss.str();
}

std::string Method::mangled_name() const {
    std::stringstream ss;
    ss << "m";
    for (auto type : m_parameters) {
        ss << type->mangled_name();
    }
    return ss.str();
}

TypeType *Method::type() const {
    return nullptr;
}

void Method::set_is_generic(bool is_generic) {
    m_is_generic = is_generic;
}

bool Method::is_generic() const {
    return m_is_generic;
}

std::vector<Type *> Method::parameter_types() const {
    std::vector<Type *> parameters;
    for (int i = 1; i < m_parameters.size(); i++) {
        parameters.push_back(m_parameters[i]);
    }
    return parameters;
}

Type *Method::return_type() const {
    return m_parameters[0];
}

bool Method::could_be_called_with(std::vector<Type *> arguments) {
    auto parameters = parameter_types();

    if (arguments.size() != parameters.size()) {
        return false;
    }

    for (unsigned long i = 0; i < arguments.size(); i++) {
        if (!parameters[i]->is_compatible(arguments[i])) {
            return false;
        }
    }

    return true;
}

void Method::set_parameter_inout(Type *type, bool inout) {
    m_inouts[type] = inout;
}

bool Method::is_parameter_inout(Type *type) {
    auto it = m_inouts.find(type);
    if (it == m_inouts.end()) {
        return false;
    }

    return it->second;
}

Method *Method::clone() const {
    assert(false);
    return nullptr;
}

void Method::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Function::name() const {
    std::stringstream ss;
    ss << "Function{";
    for (auto method : m_parameters) {
        ss << method->name();
        if (method != m_parameters.back()) {
            ss << ", ";
        }
    }
    ss << "}";
    return ss.str();
}

std::string Function::mangled_name() const {
    std::stringstream ss;
    ss << "f";
    for (auto method : m_parameters) {
        ss << method->mangled_name();
    }
    return ss.str();
}

FunctionType *Function::type() const {
    return new FunctionType();
}

void Function::add_method(Method *method) {
    m_parameters.push_back(method);
}

Method *Function::find_method(ast::Node *node, std::vector<Type *> arguments) const {
    for (auto p : m_parameters) {
        auto method = dynamic_cast<Method *>(p);
        assert(method);
        if (method->could_be_called_with(arguments)) {
            return method;
        }
    }

    return nullptr;
}

Method *Function::get_method(int index) const {
    auto p = m_parameters[index];
    auto method = dynamic_cast<Method *>(p);
    assert(method);
    return method;
}

int Function::no_methods() const {
    return m_parameters.size();
}

Function *Function::clone() const {
    assert(false);
    return nullptr;
}

void Function::accept(Visitor *visitor) {
    visitor->visit(this);
}

Union::Union(Type *type1, Type *type2) {
    m_parameters.push_back(type1);
    m_parameters.push_back(type2);
}

Union::Union(std::vector<Type *> types) {
    assert(!types.empty());
    m_parameters = types;
}

std::string Union::name() const {
    std::stringstream ss;
    ss << "Union{";
    for (auto type : m_parameters) {
        ss << type->name();
        if (type != m_parameters.back()) {
            ss << ", ";
        }
    }
    ss << "}";
    return ss.str();
}

std::string Union::mangled_name() const {
    std::stringstream ss;
    ss << "u";
    for (auto type : m_parameters) {
        ss << type->mangled_name();
    }
    return ss.str();
}

UnionType *Union::type() const {
    return new UnionType();
}

std::vector<Type *> Union::types() const {
    return m_parameters;
}

uint8_t Union::type_index(const Type *type, bool *exists) const {
    for (uint8_t i = 0; i < m_parameters.size(); i++) {
        if (m_parameters[i]->is_compatible(type)) {
            *exists = true;
            return i;
        }
    }

    *exists = false;
    return 0;
}

bool Union::is_compatible(const Type *other) const {
    for (auto type : m_parameters) {
        if (type->is_compatible(other)) {
            return true;
        }
    }

    return false;
}

Union *Union::clone() const {
    std::vector<Type *> types;
    for (auto t : m_parameters) {
        types.push_back(t->clone());
    }

    return new Union(types);
}

void Union::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}
