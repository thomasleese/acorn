#include <cstring>
#include <sstream>
#include <iostream>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/typesystem/visitor.h"

#include "acorn/typesystem/types.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::typesystem;

Type::Type(ast::TypeDecl *decl_node, std::vector<Type *> parameters) :
    m_decl_node(decl_node), m_parameters(parameters) { }

bool Type::is_compatible(const Type *other) const {
    auto name1 = name();
    auto name2 = other->name();
    return name1 == name2;
}

TypeType::TypeType(ast::TypeDecl *decl_node, std::vector<TypeType *> parameters) : AbstractType(decl_node) {
    for (auto t : parameters) {
        m_parameters.push_back(t);
    }
}

std::string TypeType::mangled_name() const {
    return "u";
}

TypeType *TypeType::type() const {
    return new TypeDescriptionType(m_decl_node);
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
    return (bool) dynamic_cast<const typesystem::TypeType *>(other);
}

Type *ParameterType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Parameter(this);
    } else {
        reporter->report(InvalidTypeConstructor(node));
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

std::string VoidType::name() const {
    return "VoidType";
}

Type *VoidType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Void(this);
    } else {
        reporter->report(InvalidTypeConstructor(node));
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

Type *BooleanType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Boolean(this);
    } else {
        reporter->report(InvalidTypeConstructor(node));
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

IntegerType::IntegerType(ast::TypeDecl *decl_node, unsigned int size) :
    TypeType(decl_node), m_size(size) { }

std::string IntegerType::name() const {
    std::stringstream ss;
    ss << "IntegerType" << m_size;
    return ss.str();
}

Type *IntegerType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Integer(this, m_size);
    } else {
        reporter->report(InvalidTypeConstructor(node));
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

UnsignedIntegerType::UnsignedIntegerType(ast::TypeDecl *decl_node, unsigned int size) :
    TypeType(decl_node), m_size(size) { }

std::string UnsignedIntegerType::name() const {
    std::stringstream ss;
    ss << "UnsignedIntegerType" << m_size;
    return ss.str();
}

Type *UnsignedIntegerType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        return new UnsignedInteger(this, m_size);
    } else {
        reporter->report(InvalidTypeConstructor(node));
        return nullptr;
    }
}

UnsignedIntegerType *UnsignedIntegerType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return this;
    } else {
        return nullptr;
    }
}

void UnsignedIntegerType::accept(Visitor *visitor) {
    visitor->visit(this);
}

FloatType::FloatType(ast::TypeDecl *decl_node, int size) :
    TypeType(decl_node), m_size(size) { }

std::string FloatType::name() const {
    std::stringstream ss;
    ss << "FloatType" << m_size;
    return ss.str();
}

Type *FloatType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        return new Float(this, m_size);
    } else {
        reporter->report(InvalidTypeConstructor(node));
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

UnsafePointerType::UnsafePointerType(ast::TypeDecl *decl_node, TypeType *element_type) :
    TypeType(decl_node) {
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
    auto t = dynamic_cast<typesystem::TypeType *>(m_parameters[0]);
    assert(t);
    return t;
}

Type *UnsafePointerType::create(Reporter *reporter, ast::Node *node) {
    if (has_element_type()) {
        auto element_type_inst = element_type()->create(reporter, node);
        return new UnsafePointer(this, element_type_inst);
    } else {
        reporter->report(InvalidTypeParameters(node, m_parameters.size(), 1));
        return nullptr;
    }
}

UnsafePointerType *UnsafePointerType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return new UnsafePointerType(m_decl_node);
    } else if (parameters.size() == 1) {
        return new UnsafePointerType(m_decl_node, parameters[0]);
    } else {
        return nullptr;
    }
}

void UnsafePointerType::accept(Visitor *visitor) {
    visitor->visit(this);
}

FunctionType::FunctionType(ast::TypeDecl *node_decl, std::vector<TypeType *> parameters) :
    TypeType(node_decl, parameters) { }

std::string FunctionType::name() const {
    return "FunctionType";
}

Type *FunctionType::create(Reporter *reporter, ast::Node *node) {
    auto function = new Function(this);

    for (auto parameter : m_parameters) {
        auto method_type = dynamic_cast<MethodType *>(parameter);
        if (method_type == nullptr) {
            reporter->report(InvalidTypeParameters(node, 0, 0));
            continue;
        }

        auto method = dynamic_cast<Method *>(method_type->create(reporter, node));
        if (method == nullptr) {
            reporter->report(InvalidTypeParameters(node, 0, 0));
            continue;
        }

        function->add_method(method);
    }

    return function;
}

FunctionType *FunctionType::with_parameters(std::vector<TypeType *> parameters) {
    return new FunctionType(m_decl_node, parameters);
}

void FunctionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

MethodType::MethodType(ast::TypeDecl *decl_node, std::vector<TypeType *> parameters) :
    TypeType(decl_node, parameters) { }

std::string MethodType::name() const {
    return "MethodType";
}

Type *MethodType::create(Reporter *reporter, ast::Node *node) {
    auto return_type = dynamic_cast<TypeType *>(m_parameters[0])->create(reporter, node);
    if (return_type == nullptr) {
        return nullptr;
    }

    std::vector<Type *> parameter_types;
    for (size_t i = 1; i < m_parameters.size(); i++) {
        auto type = dynamic_cast<TypeType *>(m_parameters[i])->create(reporter, node);
        if (type == nullptr) {
            return nullptr;
        }
        parameter_types.push_back(type);
    }

    return new Method(this, parameter_types, return_type);
}

MethodType *MethodType::with_parameters(std::vector<TypeType *> parameters) {
    return new MethodType(m_decl_node, parameters);
}

void MethodType::accept(Visitor *visitor) {
    visitor->visit(this);
}

RecordType::RecordType(ast::TypeDecl *decl_node, Reporter *reporter) :
    TypeType(decl_node), m_reporter(reporter) {
    m_constructor = new Function(new FunctionType(decl_node));
}

RecordType::RecordType(ast::TypeDecl *decl_node,
                       std::vector<ParameterType *> input_parameters,
                       std::vector<std::string> field_names,
                       std::vector<TypeType *> field_types,
                       Reporter *reporter) :
    TypeType(decl_node),
    m_reporter(reporter),
    m_input_parameters(input_parameters),
    m_field_names(field_names),
    m_field_types(field_types) {
    m_constructor = new Function(new FunctionType(decl_node));
    create_builtin_constructor();
}

RecordType::RecordType(ast::TypeDecl *decl_node,
                       std::vector<ParameterType *> input_parameters,
                       std::vector<std::string> field_names,
                       std::vector<TypeType *> field_types,
                       std::vector<TypeType *> parameters,
                       Reporter *reporter) :
    TypeType(decl_node, parameters),
    m_reporter(reporter),
    m_input_parameters(input_parameters),
    m_field_names(field_names),
    m_field_types(field_types) {
    m_constructor = new Function(new FunctionType(decl_node));
    create_builtin_constructor();
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

Function *RecordType::constructor() const {
    return m_constructor;
}

TypeType *replace_parameters(TypeType *type, std::map<ParameterType *, TypeType *> mapping) {
    if (auto p = dynamic_cast<ParameterType *>(type)) {
        auto it = mapping.find(p);
        if (it != mapping.end()) {
            type = it->second;
        }
    }

    auto parameters = type->parameters();
    for (size_t i = 0; i < parameters.size(); i++) {
        auto tt = dynamic_cast<TypeType *>(parameters[i]);
        assert(tt);
        parameters[i] = replace_parameters(tt, mapping);
    }

    return type->with_parameters(parameters);
}

Type *RecordType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.size() == m_input_parameters.size()) {
        std::map<ParameterType *, TypeType *> parameter_mapping;
        for (size_t i = 0; i < m_input_parameters.size(); i++) {
            auto type_type = dynamic_cast<TypeType *>(m_parameters[i]);
            assert(type_type);
            parameter_mapping[m_input_parameters[i]] = type_type;
        }

        std::vector<Type *> field_types;

        for (size_t i = 0; i < m_field_types.size(); i++) {
            auto type = replace_parameters(m_field_types[i], parameter_mapping);

            auto result = type->create(reporter, node);
            if (result == nullptr) {
                return nullptr;
            }
            field_types.push_back(result);
        }

        return new Record(this, m_field_names, field_types);
    } else {
        reporter->report(InvalidTypeParameters(node, m_parameters.size(), m_input_parameters.size()));
        return nullptr;
    }
}

RecordType *RecordType::with_parameters(std::vector<TypeType *> parameters) {
    return new RecordType(m_decl_node, m_input_parameters, m_field_names, m_field_types, parameters, m_reporter);
}

void RecordType::accept(Visitor *visitor) {
    visitor->visit(this);
}

void RecordType::create_builtin_constructor() {
    std::vector<Type *> field_types;

    for (auto &type : m_field_types) {
        auto result = type->create(m_reporter, m_decl_node);
        field_types.push_back(result);
    }

    auto method = new Method(new MethodType(m_decl_node), field_types, create(m_reporter, m_decl_node));

    method->add_empty_specialisation();

    for (size_t i = 0; i < m_field_names.size(); i++) {
        method->set_parameter_name(i, m_field_names[i]);
    }

    m_constructor->add_method(method);
}

TupleType::TupleType(ast::TypeDecl *decl_node, diagnostics::Reporter *reporter, std::vector<TypeType *> parameters) :
    RecordType(decl_node, reporter) { }

std::string TupleType::name() const {
    return "TupleType";
}

Type *TupleType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.empty()) {
        reporter->report(InvalidTypeParameters(node, m_parameters.size(), 1));
        return nullptr;
    } else {
        std::vector<Type *> parameters;

        for (auto p : m_parameters) {
            auto tt = dynamic_cast<TypeType *>(p);
            assert(tt);
            parameters.push_back(tt->create(reporter, node));
        }

        return new Tuple(this, parameters);
    }
}

TupleType *TupleType::with_parameters(std::vector<TypeType *> parameters) {
    return new TupleType(m_decl_node, m_reporter, parameters);
}

void TupleType::accept(Visitor *visitor) {
    visitor->visit(this);
}

AliasType::AliasType(ast::TypeDecl *decl_node, TypeType *alias, std::vector<ParameterType *> input_parameters, std::vector<TypeType *> parameters) :
    TypeType(decl_node, parameters), m_alias(alias), m_input_parameters(input_parameters) { }

std::string AliasType::name() const {
    return m_alias->name();
}

Type *AliasType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.size() == m_input_parameters.size()) {
        // early escape
        if (m_input_parameters.empty()) {
            return m_alias->create(reporter, node);
        }

        std::map<ParameterType *, TypeType *> parameter_mapping;
        for (size_t i = 0; i < m_input_parameters.size(); i++) {
            auto type_type = dynamic_cast<TypeType *>(m_parameters[i]);
            assert(type_type);
            parameter_mapping[m_input_parameters[i]] = type_type;
        }

        auto alias = replace_parameters(m_alias, parameter_mapping);
        return alias->create(reporter, node);
    } else {
        reporter->report(InvalidTypeParameters(node, m_parameters.size(), m_input_parameters.size()));
        return nullptr;
    }
}

AliasType *AliasType::with_parameters(std::vector<TypeType *> parameters) {
    return new AliasType(m_decl_node, m_alias, m_input_parameters, parameters);
}

void AliasType::accept(Visitor *visitor) {
    visitor->visit(this);
}

ModuleType::ModuleType(ast::TypeDecl *decl_node) :
    AbstractType(decl_node) { }

std::string ModuleType::name() const {
    return "ModuleType";
}

std::string ModuleType::mangled_name() const {
    return "mod";
}

TypeType *ModuleType::type() const {
    return nullptr;
}

ModuleType *ModuleType::with_parameters(std::vector<Type *> parameters) {
    return nullptr;
}

void ModuleType::accept(Visitor *visitor) {
    visitor->visit(this);
}

TypeDescriptionType::TypeDescriptionType(ast::TypeDecl *decl_node, TypeType *type) : TypeType(decl_node) {
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

Type *TypeDescriptionType::create(Reporter *reporter, ast::Node *node) {
    if (m_parameters.size() == 1) {
        return m_parameters[0];
    } else {
        reporter->report(InvalidTypeParameters(node, m_parameters.size(), 1));
        return nullptr;
    }
}

TypeDescriptionType *TypeDescriptionType::with_parameters(std::vector<TypeType *> parameters) {
    if (parameters.empty()) {
        return new TypeDescriptionType(m_decl_node);
    } else if (parameters.size() == 1) {
        return new TypeDescriptionType(m_decl_node, parameters[0]);
    } else {
        return nullptr;
    }
}

void TypeDescriptionType::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(ParameterType *type) :
    AbstractType(type->decl_node()), m_type(type) { }

std::string Parameter::name() const {
    return "Parameter";
}

std::string Parameter::mangled_name() const {
    return "p";
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

Void::Void(VoidType *type) :
    Type(type->decl_node()), m_type(type) { }

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

Boolean::Boolean(BooleanType *type) :
    Type(type->decl_node()), m_type(type) { }

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

Integer::Integer(IntegerType *type, unsigned int size) :
    Type(type->decl_node()), m_type(type), m_size(size) { }

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

UnsignedInteger::UnsignedInteger(UnsignedIntegerType *type, unsigned int size) :
    Type(type->decl_node()), m_type(type), m_size(size) { }

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

Float::Float(FloatType *type, unsigned int size) :
    Type(type->decl_node()), m_type(type), m_size(size) { }

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

UnsafePointer::UnsafePointer(UnsafePointerType *type, Type *element_type) :
    Type(type->decl_node()), m_type(type) {
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
        return new UnsafePointer(m_type, parameters[0]);
    } else {
        return nullptr;
    }
}

void UnsafePointer::accept(Visitor *visitor) {
    visitor->visit(this);
}

Record::Record(RecordType *type, std::vector<std::string> field_names, std::vector<Type *> field_types) :
    Type(type->decl_node()),
    m_type(type),
    m_field_names(field_names) {
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
    for (size_t i = 0; i < m_field_names.size(); i++) {
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

bool Record::has_child(std::string name) {
    return get_field_index(name) != -1;
}

Type *Record::child_type(std::string name) {
    return get_field_type(name);
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

bool Record::is_compatible(const Type *other) const {
    auto other_record = dynamic_cast<const Record *>(other);
    if (other_record) {
        if (m_parameters.size() == other_record->m_parameters.size()) {
            for (size_t i = 0; i < m_parameters.size(); i++) {
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
    return new Record(m_type, m_field_names, parameters);
}

void Record::accept(Visitor *visitor) {
    visitor->visit(this);
}

Tuple::Tuple(TupleType *type, std::vector<Type *> field_types) : Record(type, std::vector<std::string>(), field_types) {
    for (size_t i = 0; i < field_types.size(); i++) {
        std::stringstream ss;
        ss << i;
        m_field_names.push_back(ss.str());
    }
}

void Tuple::accept(Visitor *visitor) {
    visitor->visit(this);
}

Method::Method(MethodType *type, Type *return_type) : Type(type->decl_node()), m_type(type) {
    m_parameters.push_back(return_type);
}

Method::Method(MethodType *type, std::vector<Type *> parameter_types, Type *return_type) :
    Method(type, return_type) {
    for (auto p : parameter_types) {
        assert(p);
        m_parameters.push_back(p);
    }
}

Method::Method(MethodType *type, Type *parameter1_type, Type *return_type) : Method(type, return_type) {
    m_parameters.push_back(parameter1_type);
}

Method::Method(MethodType *type, Type *parameter1_type, Type *parameter2_type, Type *return_type) : Method(type, return_type) {
    m_parameters.push_back(parameter1_type);
    m_parameters.push_back(parameter2_type);
}

Method::Method(MethodType *type, Type *parameter1_type, Type *parameter2_type, Type *parameter3_type, Type *return_type) : Method(type, return_type) {
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

std::vector<Type *> Method::parameter_types() const {
    std::vector<Type *> parameters;
    for (size_t i = 1; i < m_parameters.size(); i++) {
        parameters.push_back(m_parameters[i]);
    }
    return parameters;
}

int Method::parameter_index(std::string name) const {
    auto it = m_names.find(name);
    if (it != m_names.end()) {
        return it->second;
    } else {
        return -1;
    }
}

Type *Method::return_type() const {
    return m_parameters[0];
}

template<typename T>
std::vector<T> Method::ordered_arguments(std::vector<T> positional_arguments, std::map<std::string, T> keyword_arguments, bool *valid) {
    auto no_parameters = m_parameters.size() - 1;

    std::vector<T> ordered_arguments;

    if ((positional_arguments.size() + keyword_arguments.size()) != no_parameters) {
        if (valid) { *valid = false; }
        return ordered_arguments;
    }

    ordered_arguments.resize(no_parameters, nullptr);

    // fill in keyword arguments
    for (auto const &entry : keyword_arguments) {
        int index = parameter_index(entry.first);
        if (index == -1) {
            if (valid) { *valid = false; }
            return ordered_arguments;
        }

        ordered_arguments[index] = entry.second;
    }

    // fill in positional arguments
    int i = 0;
    for (auto &arg : positional_arguments) {
        while (ordered_arguments[i] != nullptr) {
            i++;
        }
        ordered_arguments[i] = arg;
    }

    for (auto &arg : ordered_arguments) {
        if (arg == nullptr) {
            if (valid) { *valid = false; }
        }
    }

    if (valid) { *valid = true; }
    return ordered_arguments;
}

std::vector<ast::Node *> Method::ordered_arguments(ast::Call *call, bool *valid) {
    return ordered_arguments(call->positional_arguments(), call->keyword_arguments(), valid);
}

std::vector<Type *> Method::ordered_argument_types(ast::Call *call, bool *valid) {
    return ordered_arguments(call->positional_argument_types(), call->keyword_argument_types(), valid);
}

bool Method::could_be_called_with(std::vector<Type *> positional_arguments, std::map<std::string, Type *> keyword_arguments) {
    bool valid;
    auto arguments = ordered_arguments(positional_arguments, keyword_arguments, &valid);

    if (!valid) {
        return false;
    }

    for (unsigned long i = 0; i < arguments.size(); i++) {
        bool compatible = m_parameters[i + 1]->is_compatible(arguments[i]);
        if (!compatible) {
            return false;
        }
    }

    return true;
}

void Method::add_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation) {
    m_specialisations.push_back(specialisation);
}

std::vector<std::map<typesystem::ParameterType *, typesystem::Type *> > Method::generic_specialisations() {
    return m_specialisations;
}

size_t Method::no_generic_specialisation() const {
    return m_specialisations.size();
}

void Method::add_empty_specialisation() {
    std::map<typesystem::ParameterType *, typesystem::Type *> empty_specialisation;
    add_generic_specialisation(empty_specialisation);
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

void Method::set_parameter_name(int index, std::string name) {
    m_names[name] = index;
}

Method *Method::with_parameters(std::vector<Type *> parameters) {
    assert(false);
    return nullptr;
}

void Method::accept(Visitor *visitor) {
    visitor->visit(this);
}

Function::Function(FunctionType *type) : Type(type->decl_node()), m_type(type) { }

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

void Function::add_method(Method *method) {
    m_parameters.push_back(method);
}

Method *Function::find_method(ast::Node *node, std::vector<Type *> positional_arguments, std::map<std::string, Type *> keyword_arguments) const {
    for (auto p : m_parameters) {
        auto method = dynamic_cast<Method *>(p);
        assert(method);
        if (method->could_be_called_with(positional_arguments, keyword_arguments)) {
            return method;
        }
    }

    return nullptr;
}

Method *Function::find_method(ast::Call *call) const {
    return find_method(
        call,
        call->positional_argument_types(),
        call->keyword_argument_types()
    );
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

std::vector<Method *> Function::methods() const {
    std::vector<Method *> methods;
    for (size_t i = 0; i < m_parameters.size(); i++) {
        methods.push_back(get_method(i));
    }
    return methods;
}

int Function::index_of(Method *method) const {
    for (int i = 0; i < no_methods(); i++) {
        if (get_method(i) == method) {
            return i;
        }
    }

    return -1;
}

void Function::set_llvm_index(Method *method, int index) {
    m_llvm_index[method] = index;
}

int Function::get_llvm_index(Method *method) {
    return m_llvm_index[method];
}

Function *Function::with_parameters(std::vector<Type *> parameters) {
    assert(false);
    return nullptr;
}

void Function::accept(Visitor *visitor) {
    visitor->visit(this);
}
