//
// Created by Thomas Leese on 18/03/2016.
//

#include <cstring>
#include <sstream>
#include <iostream>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>

#include "AbstractSyntaxTree.h"
#include "Errors.h"

#include "Types.h"

using namespace Types;

Type::~Type() {

}

bool Type::isCompatible(const Type *other) const {
    return name() == other->name();
}

bool Type::operator==(const Type &other) const {
    return name() == other.name();
}

Type *Constructor::create(AST::Node *node) {
    std::vector<Type *> parameters;
    return create(node, parameters);
}

std::string Constructor::mangled_name() const {
    throw std::runtime_error("cannot create mangled name");
}

llvm::Type *Constructor::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("cannot create llvm type of constructor");
}

llvm::Constant *Constructor::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("cannot create llvm initialiser of constructor");
}

std::string AnyConstructor::name() const {
    return "AnyConstructor";
}

Type *AnyConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Any();
    } else {
        throw Errors::InvalidTypeConstructor(node);
    }
}

std::string VoidConstructor::name() const {
    return "VoidConstructor";
}

Type *VoidConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Void();
    } else {
        throw Errors::InvalidTypeConstructor(node);
    }
}

std::string BooleanConstructor::name() const {
    return "BooleanConstructor";
}

Type *BooleanConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Boolean();
    } else {
        throw Errors::InvalidTypeConstructor(node);
    }
}

IntegerConstructor::IntegerConstructor(unsigned int size) : m_size(size) {

}

std::string IntegerConstructor::name() const {
    std::stringstream ss;
    ss << "IntegerConstructor" << m_size;
    return ss.str();
}

Type *IntegerConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Integer(m_size);
    } else {
        throw Errors::InvalidTypeConstructor(node);
    }
}

UnsignedIntegerConstructor::UnsignedIntegerConstructor(unsigned int size) : m_size(size) {

}

std::string UnsignedIntegerConstructor::name() const {
    std::stringstream ss;
    ss << "UnsignedIntegerConstructor" << m_size;
    return ss.str();
}

Type *UnsignedIntegerConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new UnsignedInteger(m_size);
    } else {
        throw Errors::InvalidTypeConstructor(node);
    }
}

FloatConstructor::FloatConstructor(int size) : m_size(size) {

}

std::string FloatConstructor::name() const {
    std::stringstream ss;
    ss << "FloatConstructor" << m_size;
    return ss.str();
}

Type *FloatConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Float(m_size);
    } else {
        throw Errors::InvalidTypeConstructor(node);
    }
}

std::string SequenceConstructor::name() const {
    return "SequenceConstructor";
}

Type *SequenceConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() == 1) {
        return new Sequence(parameters[0]);
    } else {
        throw Errors::InvalidTypeParameters(node);
    }
}

std::string FunctionConstructor::name() const {
    return "FunctionConstructor";
}

Type *FunctionConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    Function *function = new Function();

    for (auto parameter : parameters) {
        Method *method = dynamic_cast<Method *>(parameter);
        if (method == nullptr) {
            throw Errors::InvalidTypeParameters(node);
        }

        function->add_method(method);
    }

    return function;
}

RecordConstructor::RecordConstructor() {

}

RecordConstructor::RecordConstructor(std::vector<std::string> field_names, std::vector<Type *> field_types) :
        m_field_names(field_names), m_field_types(field_types) {

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

Type *RecordConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.empty()) {
        return new Record(m_field_names, m_field_types);
    } else {
        throw Errors::InvalidTypeParameters(node);
    }
}

std::string UnionConstructor::name() const {
    return "UnionConstructor";
}

Type *UnionConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    std::set<Type *> types;
    for (auto parameter : parameters) {
        types.insert(parameter);
    }

    return new Union(node, types);
}

AliasConstructor::AliasConstructor(AST::Node *node, Constructor *constructor, std::vector<Type *> inputParameters, std::vector<Type *> outputParameters) :
        m_constructor(constructor) {
    for (unsigned long i = 0; i < inputParameters.size(); i++) {
        auto parameter = dynamic_cast<Parameter *>(inputParameters[i]);
        if (parameter == nullptr) {
            throw Errors::InvalidTypeParameters(node);
        }

        int mapping = -1;

        for (unsigned long j = 0; j < outputParameters.size(); j++) {
            if (*parameter == *(outputParameters[j])) {
                mapping = j;
                break;
            }
        }

        if (mapping == -1) {
            throw Errors::InvalidTypeParameters(node);
        }

        m_parameterMapping[i] = mapping;
    }

    for (auto t : outputParameters) {
        if (dynamic_cast<Parameter *>(t) == nullptr) {
            m_knownTypes.push_back(t);
        }
    }
}

std::string AliasConstructor::name() const {
    return "AliasConstructor";
}

Type *AliasConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    if (parameters.size() != m_parameterMapping.size()) {
        throw Errors::InvalidTypeParameters(node);
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

    return m_constructor->create(node, actualParameters);
}

Parameter::Parameter(std::string name) : m_name(name) {

}

std::string Parameter::name() const {
    return m_name;
}

std::string Parameter::mangled_name() const {
    return "p";
}

llvm::Type *Parameter::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

llvm::Constant *Parameter::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

std::string Any::name() const {
    return "Any";
}

std::string Any::mangled_name() const {
    return "a";
}

llvm::Type *Any::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

llvm::Constant *Any::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

std::string Void::name() const {
    return "Void";
}

std::string Void::mangled_name() const {
    return "v";
}

llvm::Type *Void::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getVoidTy(context);
}

llvm::Constant *Void::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

std::string Boolean::name() const {
    return "Boolean";
}

std::string Boolean::mangled_name() const {
    return "b";
}

llvm::Type *Boolean::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getInt1Ty(context);
}

llvm::Constant *Boolean::create_llvm_initialiser(llvm::LLVMContext &context) const {
    return llvm::ConstantInt::get(create_llvm_type(context), 0);
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

llvm::Type *Integer::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getIntNTy(context, m_size);
}

llvm::Constant *Integer::create_llvm_initialiser(llvm::LLVMContext &context) const {
    return llvm::ConstantInt::get(create_llvm_type(context), 0);
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

llvm::Type *UnsignedInteger::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getIntNTy(context, m_size);
}

llvm::Constant *UnsignedInteger::create_llvm_initialiser(llvm::LLVMContext &context) const {
    return llvm::ConstantInt::get(create_llvm_type(context), 0);
}

Float::Float(int size) : m_size(size) {

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

llvm::Type *Float::create_llvm_type(llvm::LLVMContext &context) const {
    switch (m_size) {
        case 64:
            return llvm::Type::getDoubleTy(context);
        case 32:
            return llvm::Type::getFloatTy(context);
        case 16:
            return llvm::Type::getHalfTy(context);
        default:
            throw Errors::InternalError(static_cast<Token *>(nullptr), "Invalid float type.");
    }
}

llvm::Constant *Float::create_llvm_initialiser(llvm::LLVMContext &context) const {
    return llvm::ConstantFP::get(create_llvm_type(context), 0);
}

Sequence::Sequence(Type *elementType) {
    m_elementType = elementType;
}

std::string Sequence::name() const {
    std::stringstream ss;
    ss << "Sequence{" << m_elementType->name() << "}";
    return ss.str();
}

std::string Sequence::mangled_name() const {
    std::stringstream ss;
    ss << "s" << m_elementType->mangled_name();
    return ss.str();
}

llvm::Type *Sequence::create_llvm_type(llvm::LLVMContext &context) const {
    llvm::Type *elementType = m_elementType->create_llvm_type(context);
    return llvm::ArrayType::get(elementType, 10);
}

llvm::Constant *Sequence::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
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

llvm::Type *Record::create_llvm_type(llvm::LLVMContext &context) const {
    std::vector<llvm::Type *> llvm_types;
    for (auto type : m_field_types) {
        llvm_types.push_back(type->create_llvm_type(context));
    }
    return llvm::StructType::get(context, llvm_types);
}

llvm::Constant *Record::create_llvm_initialiser(llvm::LLVMContext &context) const {
    std::vector<llvm::Constant *> constants;
    for (auto type : m_field_types) {
        constants.push_back(type->create_llvm_initialiser(context));
    }

    llvm::StructType *type = static_cast<llvm::StructType *>(create_llvm_type(context));
    return llvm::ConstantStruct::get(type, constants);
}

std::string Product::name() const {
    return "Product";
}

std::string Product::mangled_name() const {
    return "pr";
}

llvm::Type *Product::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

llvm::Constant *Product::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
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

bool Method::could_be_called_with(std::vector<AST::Argument *> arguments) {
    unsigned long matches = 0;

    for (unsigned long i = 0; i < arguments.size(); i++) {
        Type *type = arguments[i]->type;
        if (!type) {
            return false;
        }

        unsigned long index = i;
        if (arguments[i]->name) {
            std::string name = arguments[i]->name->name;
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

llvm::Type *Method::create_llvm_type(llvm::LLVMContext &context) const {
    llvm::Type *returnType = m_return_type->create_llvm_type(context);
    std::vector<llvm::Type *> paramTypes;
    for (auto type : m_parameter_types) {
        paramTypes.push_back(type->create_llvm_type(context));
    }

    return llvm::FunctionType::get(returnType, paramTypes, false);
}

llvm::Constant *Method::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
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

Method *Function::find_method(AST::Node *node, std::vector<AST::Argument *> arguments) const {
    for (auto method : m_methods) {
        if (method->could_be_called_with(arguments)) {
            return method;
        }
    }

    std::stringstream ss;
    for (auto arg : arguments) {
        ss << arg->type->name() << ", ";
    }

    throw Errors::UndefinedError(node, "Method with arguments " + ss.str());
}

Method *Function::get_method(int index) const {
    return m_methods[index];
}

int Function::no_methods() const {
    return m_methods.size();
}

llvm::Type *Function::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("functions do not map to LLVM, use methods instead");
}

llvm::Constant *Function::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("functions to not map to LLVM");
}

Union::Union(Type *type1, Type *type2) {
    m_types.insert(type1);
    m_types.insert(type2);
}

Union::Union(AST::Node *node, std::set<Type *> types) : m_types(types) {
    if (m_types.size() <= 1) {
        throw Errors::InvalidTypeParameters(node);
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

llvm::Type *Union::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

llvm::Constant *Union::create_llvm_initialiser(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}