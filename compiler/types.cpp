//
// Created by Thomas Leese on 18/03/2016.
//

#include <cstring>
#include <sstream>
#include <iostream>

#include "ast/nodes.h"
#include "compiler/pass.h"
#include "errors.h"

#include "types.h"

using namespace jet;
using namespace jet::types;

Type::~Type() {

}

bool Type::isCompatible(const Type *other) const {
    return name() == other->name();
}

std::string Parameter::name() const {
    return "Parameter";
}

std::string Parameter::mangled_name() const {
    return "p";
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

Type *Constructor::create(compiler::Pass *pass, ast::Node *node) {
    return create(pass, node, std::vector<Type *>());
}

std::string Constructor::mangled_name() const {
    return "c";
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

void RecordConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string UnionConstructor::name() const {
    return "UnionConstructor";
}

Type *UnionConstructor::create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) {
    std::set<Type *> types;
    for (auto parameter : parameters) {
        types.insert(parameter);
    }

    return new Union(pass, node, types);
}

void UnionConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

AliasConstructor::AliasConstructor(ast::Node *node, Constructor *constructor, std::vector<Parameter *> input_parameters, std::vector<Type *> outputParameters) :
        m_constructor(constructor), m_input_parameters(input_parameters), m_output_parameters(outputParameters) {

}

std::string AliasConstructor::name() const {
    return "AliasConstructor";
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

    return m_constructor->create(pass, node, parameters);
}

void AliasConstructor::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Any::name() const {
    return "Any";
}

std::string Any::mangled_name() const {
    return "a";
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

void Void::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Boolean::name() const {
    return "Boolean";
}

std::string Boolean::mangled_name() const {
    return "b";
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

unsigned int Integer::size() const {
    return m_size;
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

unsigned int UnsignedInteger::size() const {
    return m_size;
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

unsigned int Float::size() const {
    return m_size;
}

void Float::accept(Visitor *visitor) {
    visitor->visit(this);
}

UnsafePointer::UnsafePointer(Type *element_type) {
    m_element_type = element_type;
}

std::string UnsafePointer::name() const {
    std::stringstream ss;
    ss << "UnsafePointer{" << m_element_type->name() << "}";
    return ss.str();
}

std::string UnsafePointer::mangled_name() const {
    std::stringstream ss;
    ss << "p" << m_element_type->mangled_name();
    return ss.str();
}

Type *UnsafePointer::element_type() const {
    return m_element_type;
}

void UnsafePointer::accept(Visitor *visitor) {
    visitor->visit(this);
}

Record::Record(std::vector<std::string> field_names, std::vector<Type *> field_types) :
        m_field_names(field_names), m_field_types(field_types) {

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

    return m_field_types[index];
}

std::vector<Type *> Record::field_types() const {
    return m_field_types;
}

std::string Record::name() const {
    std::stringstream ss;
    ss << "Record{";
    for (auto type : m_field_types) {
        ss << type->name() << ", ";
    }
    return ss.str();
}

std::string Record::mangled_name() const {
    std::stringstream ss;
    ss << "r";
    for (auto type : m_field_types) {
        ss << type->mangled_name();
    }
    return ss.str();
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

Method::Method(std::vector<Type *> parameter_types, Type *return_type, std::vector<std::string> official_parameter_order) :
        m_parameter_types(parameter_types),
        m_return_type(return_type),
        m_official_parameter_order(official_parameter_order) {

}

Method::Method(Type *return_type) : m_return_type(return_type) {

}

Method::Method(std::string parameter1_name, Type *parameter1_type, Type *return_type) {
    m_parameter_types.push_back(parameter1_type);
    m_official_parameter_order.push_back(parameter1_name);
    m_return_type = return_type;
}

Method::Method(std::string parameter1_name, Type *parameter1_type, std::string parameter2_name, Type *parameter2_type, Type *return_type) {
    m_parameter_types.push_back(parameter1_type);
    m_parameter_types.push_back(parameter2_type);
    m_official_parameter_order.push_back(parameter1_name);
    m_official_parameter_order.push_back(parameter2_name);
    m_return_type = return_type;
}

std::string Method::name() const {
    std::stringstream ss;
    ss << "Method{";
    for (auto type : m_parameter_types) {
        ss << type->name() << ", ";
    }
    for (auto name : m_official_parameter_order) {
        ss << name << ", ";
    }
    ss << m_return_type->name();
    ss << "}" << ", ";
    return ss.str();
}

std::string Method::mangled_name() const {
    std::stringstream ss;
    ss << "m";
    for (auto type : m_parameter_types) {
        ss << type->mangled_name();
    }
    ss << m_return_type->mangled_name();
    return ss.str();
}

std::vector<Type *> Method::parameter_types() const {
    return m_parameter_types;
}

Type *Method::return_type() const {
    return m_return_type;
}

long Method::get_parameter_position(std::string name) const {
    auto it = std::find(m_official_parameter_order.begin(), m_official_parameter_order.end(), name);
    if (it == m_official_parameter_order.end()) {
        return -1;
    }

    return std::distance(m_official_parameter_order.begin(), it);
}

std::string Method::get_parameter_name(long position) const {
    return m_official_parameter_order[position];
}

bool Method::could_be_called_with(std::vector<ast::Argument *> arguments) {
    unsigned long matches = 0;

    for (unsigned long i = 0; i < arguments.size(); i++) {
        Type *type = arguments[i]->type;
        if (!type) {
            return false;
        }

        unsigned long index = i;
        if (arguments[i]->name) {
            std::string name = arguments[i]->name->value;
            auto pos = get_parameter_position(name);
            if (pos < 0) {
                return false;
            }

            index = pos;
        }

        if (index >= m_parameter_types.size()) {
            return false;
        }

        if (m_parameter_types[index]->isCompatible(type)) {
            matches++;
        }
    }

    // FIXME deal with default values
    return matches == m_parameter_types.size();
}

void Method::accept(Visitor *visitor) {
    visitor->visit(this);
}

std::string Function::name() const {
    std::stringstream ss;
    ss << "Function{";
    for (auto method : m_methods) {
        ss << method->name() << ", ";
    }
    ss << "}";
    return ss.str();
}

std::string Function::mangled_name() const {
    std::stringstream ss;
    ss << "f";
    for (auto method : m_methods) {
        ss << method->mangled_name();
    }
    return ss.str();
}

void Function::add_method(Method *method) {
    m_methods.push_back(method);
}

Method *Function::find_method(ast::Node *node, std::vector<ast::Argument *> arguments) const {
    for (auto method : m_methods) {
        if (method->could_be_called_with(arguments)) {
            return method;
        }
    }

    std::stringstream ss;
    for (auto arg : arguments) {
        ss << arg->type->name() << ", ";
    }

    return nullptr;
}

Method *Function::get_method(int index) const {
    return m_methods[index];
}

int Function::no_methods() const {
    return m_methods.size();
}

void Function::accept(Visitor *visitor) {
    visitor->visit(this);
}

Union::Union(Type *type1, Type *type2) {
    m_types.insert(type1);
    m_types.insert(type2);
}

Union::Union(compiler::Pass *pass, ast::Node *node, std::set<Type *> types) : m_types(types) {
    if (m_types.size() <= 1) {
        pass->push_error(new errors::InvalidTypeParameters(node, m_types.size(), 2));
    }
}

std::string Union::name() const {
    std::stringstream ss;
    ss << "Union{";
    unsigned long i = 0;
    for (auto type : m_types) {
        ss << type->name();
        if (i < m_types.size() - 1) {
            ss << ", ";
        }
        i++;
    }
    ss << "}";
    return ss.str();
}

std::string Union::mangled_name() const {
    std::stringstream ss;
    ss << "u";
    for (auto type : m_types) {
        ss << type->mangled_name();
    }
    return ss.str();
}

std::set<Type *> Union::types() const {
    return m_types;
}

bool Union::isCompatible(const Type *other) const {
    for (auto type : m_types) {
        if (type->isCompatible(other)) {
            return true;
        }
    }

    return false;
}

void Union::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}
