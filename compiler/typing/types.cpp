//
// Created by Thomas Leese on 18/03/2016.
//

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>

#include "../ast/nodes.h"
#include "../pass.h"
#include "../errors.h"

#include "types.h"

using namespace acorn;
using namespace acorn::types;

Type::Type() {

}

Type::Type(std::vector<Type *> parameters) : m_parameters(parameters) {

}

Type::~Type() {

}

bool Type::is_compatible(const Type *other) const {
    auto name1 = name();
    auto name2 = other->name();
    return name1 == name2;
}

std::vector<types::Type *> Type::parameters() const {
    return m_parameters;
}

TypeType::TypeType() {

}

TypeType::TypeType(std::vector<TypeType *> parameters) {
    for (auto t : parameters) {
        m_parameters.push_back(t);
    }
}

std::string TypeType::mangled_name() const {
    return "u";
}

TypeType *TypeType::type() const {
    return new TypeDescriptionType();
}

TypeType *TypeType::with_parameters(std::vector<Type *> parameters) {
    std::vector<TypeType *> params;
    for (auto p : parameters) {
        auto tt = dynamic_cast<TypeType *>(p);
        if (tt == nullptr) {
            return nullptr;
        }
        params.push_back(tt);
    }

    return with_parameters(params);
}

std::string ParameterType::name() const {
    return "ParameterType";
}

bool ParameterType::is_compatible(const Type *other) const {
    return (bool) dynamic_cast<const types::TypeType *>(other);
}

Type *ParameterType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Parameter(this);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

ParameterType *ParameterType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
}

void ParameterType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string AnyType::name() const {
    return "AnyType";
}

Type *AnyType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Any();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

AnyType *AnyType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
}

void AnyType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string VoidType::name() const {
    return "VoidType";
}

Type *VoidType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Void();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

VoidType *VoidType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
}

void VoidType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string BooleanType::name() const {
    return "BooleanType";
}

Type *BooleanType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Boolean();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

BooleanType *BooleanType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Type *IntegerType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Integer(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

IntegerType *IntegerType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Type *UnsignedIntegerType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new UnsignedInteger(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

UnsignedIntegerType *UnsignedIntegerType::with_parameters(std::vector<TypeType *> parameters) {
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

Type *FloatType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Float(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

FloatType *FloatType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
}

void FloatType::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnsafePointerType::UnsafePointerType(TypeType *element_type) {
    if (element_type) {
        m_parameters.push_back(element_type);
    }
}

std::string UnsafePointerType::name() const {
    if (has_element_type()) {
        return "UnsafePointerType{" + element_type()->name() + "}";
    } else {
        return "UnsafePointerType{?}";
    }
}

bool UnsafePointerType::has_element_type() const {
    return m_parameters.size() == 1 && m_parameters[0] != nullptr;
}

TypeType *UnsafePointerType::element_type() const {
    auto t = dynamic_cast<types::TypeType *>(m_parameters[0]);
    assert(t);
    return t;
}

Type *UnsafePointerType::create(compiler::Pass *pass, ast::Node *node) {
    if (has_element_type()) {
        return new UnsafePointer(element_type()->create(pass, node));
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, m_parameters.size(), 1));
        return nullptr;
    }
}

UnsafePointerType *UnsafePointerType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return new UnsafePointerType();
    } else if (parameters.size() == 1) {
        return new UnsafePointerType(parameters[0]);
    } else {
        return nullptr;
    }
}

void UnsafePointerType::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string FunctionType::name() const {
    return "FunctionType";
}

Type *FunctionType::create(compiler::Pass *pass, ast::Node *node) {
    Function *function = new Function();

    for (auto parameter : m_parameters) {
        Method *method = dynamic_cast<Method *>(parameter);
        if (method == nullptr) {
            pass->push_error(new errors::InvalidTypeParameters(node, 0, 0));
            continue;
        }

        function->add_method(method);
    }

    return function;
}

FunctionType *FunctionType::with_parameters(std::vector<TypeType *> parameters) {
    return nullptr;
}

void FunctionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

RecordType::RecordType() {

}

RecordType::RecordType(std::vector<ParameterType *> input_parameters,
                       std::vector<std::string> field_names,
                       std::vector<TypeType *> field_types) :
        m_input_parameters(input_parameters),
        m_field_names(field_names),
        m_field_types(field_types)
{

}

RecordType::RecordType(std::vector<ParameterType *> input_parameters,
                       std::vector<std::string> field_names,
                       std::vector<TypeType *> field_types,
                       std::vector<TypeType *> parameters) :
        TypeType(parameters),
        m_input_parameters(input_parameters),
        m_field_names(field_names),
        m_field_types(field_types)
{

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

TypeType *replace_parameters(TypeType *type, std::map<ParameterType *, TypeType *> mapping) {
    if (auto p = dynamic_cast<ParameterType *>(type)) {
        type = mapping[p];
    }

    auto parameters = type->parameters();
    for (int i = 0; i < parameters.size(); i++) {
        auto tt = dynamic_cast<TypeType *>(parameters[i]);
        assert(tt);
        parameters[i] = replace_parameters(tt, mapping);
    }

    return type->with_parameters(parameters);
}

Type *RecordType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.size() == m_input_parameters.size()) {
        std::map<ParameterType *, TypeType *> parameter_mapping;
        for (int i = 0; i < m_input_parameters.size(); i++) {
            auto type_type = dynamic_cast<TypeType *>(m_parameters[i]);
            assert(type_type);
            parameter_mapping[m_input_parameters[i]] = type_type;
        }

        std::vector<Type *> field_types;

        for (int i = 0; i < m_field_types.size(); i++) {
            auto type = replace_parameters(m_field_types[i], parameter_mapping);

            auto result = type->create(pass, node);
            if (result == nullptr) {
                return nullptr;
            }
            field_types.push_back(result);
        }

        return new Record(m_field_names, field_types);
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, m_parameters.size(), m_input_parameters.size()));
        return nullptr;
    }
}

RecordType *RecordType::with_parameters(std::vector<TypeType *> parameters) {
    return new RecordType(m_input_parameters, m_field_names, m_field_types, parameters);
}

void RecordType::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnionType::UnionType() {

}

UnionType::UnionType(std::vector<TypeType *> parameters) : TypeType(parameters) {

}

std::string UnionType::name() const {
    return "UnionType";
}

Type *UnionType::create(compiler::Pass *pass, ast::Node *node) {
    return new Union(m_parameters);
}

UnionType *UnionType::with_parameters(std::vector<TypeType *> parameters) {
    return new UnionType(parameters);
}

void UnionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

TupleType::TupleType() {

}

TupleType::TupleType(std::vector<TypeType *> parameters) : TypeType(parameters) {

}

std::string TupleType::name() const {
    return "TupleType";
}

Type *TupleType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.empty()) {
        pass->push_error(new errors::InvalidTypeParameters(node, m_parameters.size(), 1));
        return nullptr;
    } else {
        std::vector<Type *> parameters;

        for (auto p : m_parameters) {
            auto tt = dynamic_cast<TypeType *>(p);
            assert(tt);
            parameters.push_back(tt->create(pass, node));
        }

        return new Tuple(parameters);
    }
}

TupleType *TupleType::with_parameters(std::vector<TypeType *> parameters) {
    return new TupleType(parameters);
}

void TupleType::accept(Visitor *visitor) {
    visitor->visit(this);
}

AliasType::AliasType(TypeType *alias, std::vector<ParameterType *> input_parameters) :
        m_alias(alias),
        m_input_parameters(input_parameters)
{

}

AliasType::AliasType(TypeType *alias, std::vector<ParameterType *> input_parameters, std::vector<TypeType *> parameters) :
        TypeType(parameters),
        m_alias(alias),
        m_input_parameters(input_parameters)
{

}

std::string AliasType::name() const {
    return m_alias->name();
}

Type *AliasType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.size() == m_input_parameters.size()) {
        // early escape
        if (m_input_parameters.empty()) {
            return m_alias->create(pass, node);
        }

        std::map<ParameterType *, TypeType *> parameter_mapping;
        for (int i = 0; i < m_input_parameters.size(); i++) {
            auto type_type = dynamic_cast<TypeType *>(m_parameters[i]);
            assert(type_type);
            parameter_mapping[m_input_parameters[i]] = type_type;
        }

        auto alias = replace_parameters(m_alias, parameter_mapping);
        return alias->create(pass, node);
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, m_parameters.size(), m_input_parameters.size()));
        return nullptr;
    }
}

AliasType *AliasType::with_parameters(std::vector<TypeType *> parameters) {
    return new AliasType(m_alias, m_input_parameters, parameters);
}

void AliasType::accept(Visitor *visitor) {
    visitor->visit(this);
}

ProtocolType::ProtocolType(std::vector<ParameterType *> input_parameters, std::vector<Method *> methods)
        : m_input_parameters(input_parameters), m_methods(methods)
{

}

ProtocolType::ProtocolType(std::vector<ParameterType *> input_parameters, std::vector<Method *> methods, std::vector<TypeType *> parameters)
        : TypeType(parameters), m_input_parameters(input_parameters), m_methods(methods)
{

}

std::string ProtocolType::name() const {
    return "ProtocolType";
}

bool ProtocolType::is_compatible(const Type *other) const {
    std::cout << this->name() << " " << other->name() << std::endl;
    return false;
}

Type *ProtocolType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.size() == m_input_parameters.size()) {
        return new Protocol(m_methods, this);
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, m_parameters.size(), m_input_parameters.size()));
        return nullptr;
    }
}

ProtocolType *ProtocolType::with_parameters(std::vector<TypeType *> parameters) {
    return new ProtocolType(m_input_parameters, m_methods, parameters);
}

void ProtocolType::accept(Visitor *visitor) {
    visitor->visit(this);
}

TypeDescriptionType::TypeDescriptionType(TypeType *type) {
    if (type) {
        m_parameters.push_back(type);
    }
}

std::string TypeDescriptionType::name() const {
    if (m_parameters.size() == 1) {
        return "TypeDescriptionType{" + m_parameters[0]->name() + "}";
    } else {
        return "TypeDescriptionType{?}";
    }
}

bool TypeDescriptionType::is_compatible(const Type *other) const {
    if (m_parameters.size() == 1) {
        return m_parameters[0]->is_compatible(other);
    } else {
        return false;
    }
}

Type *TypeDescriptionType::create(compiler::Pass *pass, ast::Node *node) {
    if (m_parameters.size() == 1) {
        return m_parameters[0];
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, m_parameters.size(), 1));
        return nullptr;
    }
}

TypeDescriptionType *TypeDescriptionType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return new TypeDescriptionType();
    } else if (parameters.size() == 1) {
        return new TypeDescriptionType(parameters[0]);
    } else {
        return nullptr;
    }
}

void TypeDescriptionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(ParameterType *constructor) : m_constructor(constructor) {

}

std::string Parameter::name() const {
    return "Parameter";
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

Parameter *Parameter::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Any *Any::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Void *Void::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Boolean *Boolean::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Integer *Integer::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

UnsignedInteger *UnsignedInteger::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

Float *Float::with_parameters(std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
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

UnsafePointer *UnsafePointer::with_parameters(std::vector<Type *> parameters) {
    if (parameters.size() == 1) {
        return new UnsafePointer(parameters[0]);
    } else {
        return nullptr;
    }
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

Record *Record::with_parameters(std::vector<Type *> parameters) {
    return new Record(m_field_names, parameters);
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
        assert(p);
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
        ss << type->name();
        if (type != m_parameters.back()) {
            ss << ", ";
        }
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
        bool compatible = parameters[i]->is_compatible(arguments[i]);
        std::cout << parameters[i]->name() << " ? " << arguments[i]->name() << " = " << compatible << std::endl;
        if (!compatible) {
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

Method *Method::with_parameters(std::vector<Type *> parameters) {
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

Function *Function::with_parameters(std::vector<Type *> parameters) {
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

Union *Union::with_parameters(std::vector<Type *> parameters) {
    return new Union(parameters);
}

void Union::accept(Visitor *visitor) {
    visitor->visit(this);
}

Protocol::Protocol(ProtocolType *type) : m_type(type) {

}

Protocol::Protocol(std::vector<Method *> methods, ProtocolType *type) : m_type(type) {
    for (auto method : methods) {
        m_parameters.push_back(method);
    }
}

std::string Protocol::name() const {
    std::stringstream ss;
    ss << "Protocol{";
    for (auto p : m_parameters) {
        ss << p->name();
        if (p != m_parameters.back()) {
            ss << ", ";
        }
    }
    ss << "}";
    return ss.str();
}

std::string Protocol::mangled_name() const {
    std::stringstream ss;
    ss << "pr";
    for (auto p : m_parameters) {
        ss << p->name();
    }
    return ss.str();
}

ProtocolType *Protocol::type() const {
    return m_type;
}

std::vector<Method *> Protocol::methods() const {
    std::vector<Method *> methods;
    for (auto param : m_parameters) {
        auto method = dynamic_cast<Method *>(param);
        assert(method);
        methods.push_back(method);
    }
    return methods;
}

bool Protocol::is_compatible(const Type *other) const {
    std::cout << this->name() << " " << other->name() << std::endl;
    return false;
}

Protocol *Protocol::with_parameters(std::vector<Type *> parameters) {
    std::vector<Method *> methods;
    for (auto param : parameters) {
        auto method = dynamic_cast<Method *>(param);
        assert(method);
        methods.push_back(method);
    }

    // FIXME should m_type be null here?
    return new Protocol(methods, m_type);
}

void Protocol::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}
