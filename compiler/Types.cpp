//
// Created by Thomas Leese on 18/03/2016.
//

#include <cstring>
#include <sstream>
#include <iostream>

#include <llvm/IR/DerivedTypes.h>

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

llvm::Type *Constructor::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("cannot create llvm type");
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

IntegerConstructor::IntegerConstructor(int size) : m_size(size) {

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

std::string RecordConstructor::name() const {
    return "RecordConstructor";
}

Type *RecordConstructor::create(AST::Node *node, std::vector<Type *> parameters) {
    return new Record();
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

llvm::Type *Parameter::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

std::string Any::name() const {
    return "Any";
}

llvm::Type *Any::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

std::string Void::name() const {
    return "Void";
}

llvm::Type *Void::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getVoidTy(context);
}

std::string Boolean::name() const {
    return "Boolean";
}

llvm::Type *Boolean::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getInt8Ty(context);
}

Integer::Integer(int size) : m_size(size) {

}

std::string Integer::name() const {
    std::stringstream ss;
    ss << "Integer" << m_size;
    return ss.str();
}

llvm::Type *Integer::create_llvm_type(llvm::LLVMContext &context) const {
    return llvm::Type::getIntNTy(context, m_size);
}

Float::Float(int size) : m_size(size) {

}

std::string Float::name() const {
    std::stringstream ss;
    ss << "Float" << m_size;
    return ss.str();
}

llvm::Type *Float::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

Sequence::Sequence(Type *elementType) {
    m_elementType = elementType;
}

std::string Sequence::name() const {
    std::stringstream ss;
    ss << "Sequence{" << m_elementType->name() << "}";
    return ss.str();
}

llvm::Type *Sequence::create_llvm_type(llvm::LLVMContext &context) const {
    llvm::Type *elementType = m_elementType->create_llvm_type(context);
    return llvm::ArrayType::get(elementType, 10);
}

std::string Product::name() const {
    return "Product";
}

llvm::Type *Product::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
}

Method::Method(std::map<std::string, Type *> parameter_types, Type *return_type) :
        m_parameter_types(parameter_types),
        m_return_type(return_type) {

}

std::string Method::name() const {
    std::stringstream ss;
    ss << "Method{";
    for (auto it = m_parameter_types.begin(); it != m_parameter_types.end(); it++) {
        ss << (it->second)->name() << ", ";
    }
    ss << m_return_type->name();
    ss << "}" << ", ";
    return ss.str();
}

Type *Method::return_type() const {
    return m_return_type;
}

bool Method::could_be_called_with(std::map<std::string, Type *> parameters) {
    unsigned long matches = 0;

    for (auto it = parameters.begin(); it != parameters.end(); it++) {
        std::string name = it->first;
        Type *type = it->second;

        if (m_parameter_types[name]->isCompatible(type)) {
            matches++;
        }
    }

    // FIXME deal with default values
    return matches == m_parameter_types.size();
}

llvm::Type *Method::create_llvm_type(llvm::LLVMContext &context) const {
    llvm::Type *returnType = m_return_type->create_llvm_type(context);
    std::vector<llvm::Type *> paramTypes;
    for (auto it = m_parameter_types.begin(); it != m_parameter_types.end(); it++) {
        paramTypes.push_back(it->second->create_llvm_type(context));
    }

    return llvm::FunctionType::get(returnType, paramTypes, false);
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

void Function::add_method(Method *method) {
    m_methods.push_back(method);
}

Method *Function::find_method(AST::Node *node, std::map<std::string, Type *> parameters) const {
    for (auto method : m_methods) {
        if (method->could_be_called_with(parameters)) {
            return method;
        }
    }

    throw Errors::UndefinedError(node, "method");
}

llvm::Type *Function::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("functions do not map to LLVM, use methods instead");
}

std::string Record::name() const {
    return "Record";
}

llvm::Type *Record::create_llvm_type(llvm::LLVMContext &context) const {
    throw std::runtime_error("not implemented");
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
