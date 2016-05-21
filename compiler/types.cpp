//
// Created by Thomas Leese on 18/03/2016.
//

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>

#include "ast/nodes.h"
#include "compiler/pass.h"
#include "errors.h"

#include "types.h"

using namespace acorn;
using namespace acorn::types;

Type::~Type() {

}

bool Type::isCompatible(const Type *other) const {
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

Type *Constructor::create(compiler::Pass *pass, ast::Node *node) {
    return create(pass, node, std::vector<Type *>());
}

std::string Constructor::mangled_name() const {
    return "c";
}

std::string ParameterConstructor::name() const {
    return "ParameterConstructor";
}

bool ParameterConstructor::isCompatible(const Type *other) const {
    return (bool) dynamic_cast<const types::Constructor *>(other);
}

Type *ParameterConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Parameter(this);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

ParameterConstructor *ParameterConstructor::clone() const {
    return new ParameterConstructor();
}

void ParameterConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string AnyConstructor::name() const {
    return "AnyConstructor";
}

Type *AnyConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Any();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

AnyConstructor *AnyConstructor::clone() const {
    return new AnyConstructor();
}

void AnyConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string VoidConstructor::name() const {
    return "VoidConstructor";
}

Type *VoidConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Void();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

VoidConstructor *VoidConstructor::clone() const {
    return new VoidConstructor();
}

void VoidConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string BooleanConstructor::name() const {
    return "BooleanConstructor";
}

Type *BooleanConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Boolean();
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

BooleanConstructor *BooleanConstructor::clone() const {
    return new BooleanConstructor();
}

void BooleanConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

IntegerConstructor::IntegerConstructor(unsigned int size) : m_size(size) {

}

std::string IntegerConstructor::name() const {
    std::stringstream ss;
    ss << "IntegerConstructor" << m_size;
    return ss.str();
}

Type *IntegerConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Integer(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

IntegerConstructor *IntegerConstructor::clone() const {
    return new IntegerConstructor(m_size);
}

void IntegerConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnsignedIntegerConstructor::UnsignedIntegerConstructor(unsigned int size) : m_size(size) {

}

std::string UnsignedIntegerConstructor::name() const {
    std::stringstream ss;
    ss << "UnsignedIntegerConstructor" << m_size;
    return ss.str();
}

Type *UnsignedIntegerConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new UnsignedInteger(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

UnsignedIntegerConstructor *UnsignedIntegerConstructor::clone() const {
    return new UnsignedIntegerConstructor(m_size);
}

void UnsignedIntegerConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

FloatConstructor::FloatConstructor(int size) : m_size(size) {

}

std::string FloatConstructor::name() const {
    std::stringstream ss;
    ss << "FloatConstructor" << m_size;
    return ss.str();
}

Type *FloatConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Float(m_size);
    } else {
        pass->push_error(new errors::InvalidTypeConstructor(node));
        return nullptr;
    }
}

FloatConstructor *FloatConstructor::clone() const {
    return new FloatConstructor(m_size);
}

void FloatConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string UnsafePointerConstructor::name() const {
    return "UnsafePointerConstructor";
}

Type *UnsafePointerConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() == 1 && parameters[0] != nullptr) {
        return new UnsafePointer(parameters[0]);
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    }
}

UnsafePointerConstructor *UnsafePointerConstructor::clone() const {
    return new UnsafePointerConstructor();
}

void UnsafePointerConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string FunctionConstructor::name() const {
    return "FunctionConstructor";
}

Type *FunctionConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
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

FunctionConstructor *FunctionConstructor::clone() const {
    return new FunctionConstructor();
}

void FunctionConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

RecordConstructor::RecordConstructor() {

}

RecordConstructor::RecordConstructor(std::vector<Parameter *> input_parameters, std::vector<std::string> field_names, std::vector<Constructor *> field_types, std::vector<std::vector<Type *> > field_parameters) :
        m_input_parameters(input_parameters), m_field_names(field_names), m_field_types(field_types), m_field_parameters(field_parameters) {

}

std::string RecordConstructor::name() const {
    std::stringstream ss;
    ss << "RecordConstructor{";
    for (auto type : m_field_types) {
        ss << type->name() << ",";
    }
    ss << "}";
    return ss.str();
}

Type *RecordConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
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

RecordConstructor *RecordConstructor::clone() const {
    std::vector<Parameter *> input_parameters;
    for (auto element : m_input_parameters) {
        input_parameters.push_back(element->clone());
    }

    std::vector<Constructor *> field_types;
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

    return new RecordConstructor(input_parameters, m_field_names, field_types, field_parameters);
}

void RecordConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string UnionConstructor::name() const {
    return "UnionConstructor";
}

Type *UnionConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    } else {
        return new Union(parameters);
    }
}

UnionConstructor *UnionConstructor::clone() const {
    return new UnionConstructor();
}

void UnionConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

AliasConstructor::AliasConstructor(Constructor *constructor, std::vector<Parameter *> input_parameters, std::vector<Type *> output_parameters) :
        m_constructor(constructor),
        m_input_parameters(input_parameters),
        m_output_parameters(output_parameters)
{

}

std::string AliasConstructor::name() const {
    return m_constructor->name();
}

Type *AliasConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    for (unsigned long i = 0; i < m_input_parameters.size(); i++) {
        auto parameter = m_input_parameters[i];

        int mapping = -1;

        for (unsigned long j = 0; j < m_output_parameters.size(); j++) {
            if (parameter->isCompatible(m_output_parameters[j])) {
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

AliasConstructor *AliasConstructor::clone() const {
    std::vector<Parameter *> input_parameters;
    for (auto p : m_input_parameters) {
        input_parameters.push_back(p->clone());
    }

    std::vector<Type *> output_parameters;
    for (auto p : m_output_parameters) {
        output_parameters.push_back(p->clone());
    }

    return new AliasConstructor(m_constructor->clone(), input_parameters, output_parameters);
}

void AliasConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string TypeDescriptionConstructor::name() const {
    return "TypeDescriptionConstructor";
}

Type *TypeDescriptionConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() == 1) {
        auto ct = dynamic_cast<types::ConstructedType *>(parameters[0]);
        assert(ct);
        return ct->constructor();
    } else {
        pass->push_error(new errors::InvalidTypeParameters(node, parameters.size(), 1));
        return nullptr;
    }
}

TypeDescriptionConstructor *TypeDescriptionConstructor::clone() const {
    return new TypeDescriptionConstructor();
}

void TypeDescriptionConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(ParameterConstructor *constructor) : m_constructor(constructor) {

}

std::string Parameter::name() const {
    return "Parameter{Any}";
}

std::string Parameter::mangled_name() const {
    return "p";
}

ParameterConstructor *Parameter::constructor() const {
    return m_constructor;
}

bool Parameter::isCompatible(const Type *other) const {
    return true; // acts like Any
}

Parameter *Parameter::clone() const {
    // don't clone constructor
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

Constructor *Any::constructor() const {
    return new AnyConstructor();
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

Constructor *Void::constructor() const {
    return new VoidConstructor();
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

Constructor *Boolean::constructor() const {
    return new BooleanConstructor();
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

Constructor *Integer::constructor() const {
    return new IntegerConstructor(m_size);
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

Constructor *UnsignedInteger::constructor() const {
    return new UnsignedIntegerConstructor(m_size);
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

Constructor *Float::constructor() const {
    return new FloatConstructor(m_size);
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

Constructor *UnsafePointer::constructor() const {
    return new UnsafePointerConstructor();
}

Type *UnsafePointer::element_type() const {
    assert(m_parameters.size() == 1);
    return m_parameters[0];
}

bool UnsafePointer::isCompatible(const Type *other) const {
    auto other_pointer = dynamic_cast<const UnsafePointer *>(other);
    if (other_pointer) {
        return element_type()->isCompatible(other_pointer->element_type());
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

Constructor *Record::constructor() const {
    return new RecordConstructor();
}

bool Record::isCompatible(const Type *other) const {
    auto other_record = dynamic_cast<const Record *>(other);
    if (other_record) {
        if (m_parameters.size() == other_record->m_parameters.size()) {
            for (int i = 0; i < m_parameters.size(); i++) {
                if (!m_parameters[i]->isCompatible(other_record->m_parameters[i])) {
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

Constructor *Method::constructor() const {
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
        if (!parameters[i]->isCompatible(arguments[i])) {
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

Constructor *Function::constructor() const {
    return nullptr;
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

Constructor *Union::constructor() const {
    return new UnionConstructor();
}

std::vector<Type *> Union::types() const {
    return m_parameters;
}

uint8_t Union::type_index(const Type *type, bool *exists) const {
    for (uint8_t i = 0; i < m_parameters.size(); i++) {
        if (m_parameters[i]->isCompatible(type)) {
            *exists = true;
            return i;
        }
    }

    *exists = false;
    return 0;
}

bool Union::isCompatible(const Type *other) const {
    for (auto type : m_parameters) {
        if (type->isCompatible(other)) {
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
