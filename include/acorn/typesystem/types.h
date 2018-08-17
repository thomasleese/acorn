#pragma once

#include <set>
#include <string>
#include <vector>
#include <map>

namespace acorn {
    namespace ast {
        class Node;
        class Node;
        class Call;
        class TypeDecl;
    }

    namespace diagnostics {
        class Reporter;
    }
}

namespace acorn::typesystem {

    class Visitor;
    class TypeType;

    class Type {

    public:
        explicit Type(ast::TypeDecl *m_decl_node, std::vector<Type *> parameters = {});
        virtual ~Type() = default;

        ast::TypeDecl *decl_node() const {
            return m_decl_node;
        }

        virtual std::string name() const = 0;
        virtual std::string mangled_name() const = 0;

        virtual bool is_abstract() const {
            for (auto &type : m_parameters) {
                if (type->is_abstract()) {
                    return true;
                }
            }

            return false;
        }

        virtual bool is_compatible(const Type *other) const;

        virtual TypeType *type() const = 0;

        virtual Type *with_parameters(std::vector<Type *> parameters) = 0;

        std::vector<Type *> parameters() const {
            return m_parameters;
        }

        virtual void accept(Visitor *visitor) = 0;

    protected:
        ast::TypeDecl *m_decl_node;
        std::vector<Type *> m_parameters;
    };

    class AbstractType : public Type {
    public:
        using Type::Type;

        bool is_abstract() const {
            return true;
        }
    };

    // type "type"s -- i.e. the type of concrete typesystem
    class TypeType : public AbstractType {
    public:
        explicit TypeType(ast::TypeDecl *decl_node, std::vector<TypeType *> parameters = {});
        virtual Type *create(diagnostics::Reporter *reporter, ast::Node *node) = 0;

        std::string mangled_name() const;

        virtual TypeType *type() const;

        TypeType *with_parameters(std::vector<Type *> parameters);
        virtual TypeType *with_parameters(std::vector<TypeType *> parameters) = 0;
    };

    class ParameterType : public TypeType {
    public:
        using TypeType::TypeType;

        std::string name() const;

        bool is_compatible(const Type *other) const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        ParameterType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class VoidType : public TypeType {
    public:
        using TypeType::TypeType;

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        VoidType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class BooleanType : public TypeType {
    public:
        using TypeType::TypeType;

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        BooleanType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class IntegerType : public TypeType {
    public:
        IntegerType(ast::TypeDecl *decl_node, unsigned int size);

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        IntegerType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        unsigned int m_size;
    };

    class UnsignedIntegerType : public TypeType {
    public:
        UnsignedIntegerType(ast::TypeDecl *decl_node, unsigned int size);

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        UnsignedIntegerType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        unsigned int m_size;
    };

    class FloatType : public TypeType {
    public:
        FloatType(ast::TypeDecl *decl_node, int size);

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        FloatType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        int m_size;
    };

    class UnsafePointerType : public TypeType {
    public:
        explicit UnsafePointerType(ast::TypeDecl *decl_node, TypeType *element_type = nullptr);

        std::string name() const;

        bool has_element_type() const;
        TypeType *element_type() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        UnsafePointerType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class FunctionType : public TypeType {
    public:
        explicit FunctionType(ast::TypeDecl *decl_node, std::vector<TypeType *> parameters = {});

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        FunctionType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class MethodType : public TypeType {
    public:
        explicit MethodType(ast::TypeDecl *decl_node, std::vector<TypeType *> parameters = {});

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        MethodType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class Function;

    class RecordType : public TypeType {

    public:
        RecordType(ast::TypeDecl *decl_node, diagnostics::Reporter *reporter);
        RecordType(ast::TypeDecl *decl_node, std::vector<ParameterType *> input_parameters,
                   std::vector<std::string> field_names,
                   std::vector<TypeType *> field_types,
                   diagnostics::Reporter *reporter);
        RecordType(ast::TypeDecl *decl_node, std::vector<ParameterType *> input_parameters,
                   std::vector<std::string> field_names,
                   std::vector<TypeType *> field_types,
                   std::vector<TypeType *> parameters,
                   diagnostics::Reporter *reporter);

        std::string name() const;

        Function *constructor() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        RecordType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        void create_builtin_constructor();

    protected:
        diagnostics::Reporter *m_reporter;

        std::vector<ParameterType *> m_input_parameters;
        std::vector<std::string> m_field_names;
        std::vector<TypeType *> m_field_types;
        Function *m_constructor;

    };

    class TupleType : public RecordType {
    public:
        explicit TupleType(ast::TypeDecl *decl_node, diagnostics::Reporter *reporter, std::vector<TypeType *> parameters = {});

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        TupleType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    class AliasType : public TypeType {

    public:
        AliasType(ast::TypeDecl *decl_node, TypeType *alias, std::vector<ParameterType *> input_parameters, std::vector<TypeType *> parameters = {});

        std::string name() const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        AliasType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);

    private:
        TypeType *m_alias;
        std::vector<ParameterType *> m_input_parameters;
        std::map<int, int> m_parameterMapping;
        std::vector<Type *> m_knownTypes;

    };

    class ModuleType : public AbstractType {
    public:
        ModuleType(ast::TypeDecl *decl_node);

        std::string name() const;
        std::string mangled_name() const;

        TypeType *type() const;
        ModuleType *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);
    };

    class TypeDescriptionType : public TypeType {
    public:
        explicit TypeDescriptionType(ast::TypeDecl *decl_node, TypeType *type = nullptr);

        std::string name() const;

        bool is_compatible(const Type *other) const;

        Type *create(diagnostics::Reporter *reporter, ast::Node *node);

        TypeDescriptionType *with_parameters(std::vector<TypeType *> parameters);

        void accept(Visitor *visitor);
    };

    // constructed typesystem
    class Parameter : public AbstractType {

    public:
        Parameter(ParameterType *type);

        std::string name() const override;
        std::string mangled_name() const override;

        ParameterType *type() const override {
            return m_type;
        }

        bool is_compatible(const Type *other) const override;

        Parameter *with_parameters(std::vector<Type *> parameters) override;

        void accept(Visitor *visitor) override;

    private:
        ParameterType *m_type;

    };

    class Void : public Type {
    public:
        explicit Void(VoidType *type);

        std::string name() const {
            return "Void";
        }

        std::string mangled_name() const {
            return "v";
        }

        VoidType *type() const {
            return m_type;
        }

        Void *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        VoidType *m_type;
    };

    class Boolean : public Type {
    public:
        explicit Boolean(BooleanType *type);

        std::string name() const {
            return "Boolean";
        }

        std::string mangled_name() const {
            return "b";
        }

        BooleanType *type() const {
            return m_type;
        }

        Boolean *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        BooleanType *m_type;
    };

    class Integer : public Type {
    public:
        Integer(IntegerType *type, unsigned int size);

        std::string name() const;
        std::string mangled_name() const;

        IntegerType *type() const {
            return m_type;
        }

        unsigned int size() const {
            return m_size;
        }

        Integer *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        IntegerType *m_type;
        unsigned int m_size;
    };

    class UnsignedInteger : public Type {
    public:
        UnsignedInteger(UnsignedIntegerType *type, unsigned int size);

        std::string name() const;
        std::string mangled_name() const;

        UnsignedIntegerType *type() const {
            return m_type;
        }

        unsigned int size() const {
            return m_size;
        }

        UnsignedInteger *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        UnsignedIntegerType *m_type;
        unsigned int m_size;
    };

    class Float : public Type {
    public:
        Float(FloatType *type, unsigned int size);

        std::string name() const;
        std::string mangled_name() const;

        FloatType *type() const {
            return m_type;
        }

        unsigned int size() const {
            return m_size;
        }

        Float *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        FloatType *m_type;
        unsigned int m_size;
    };

    class UnsafePointer : public Type {
    public:
        UnsafePointer(UnsafePointerType *type, Type *element_type);

        std::string name() const;
        std::string mangled_name() const;

        UnsafePointerType *type() const {
            return m_type;
        }

        Type *element_type() const {
            return m_parameters[0];
        }

        bool is_compatible(const Type *other) const;

        UnsafePointer *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        UnsafePointerType *m_type;
    };

    class Record : public Type {
    public:
        Record(RecordType *type, std::vector<std::string> field_names, std::vector<Type *> field_types);

        bool has_field(std::string name);
        long get_field_index(std::string name);
        Type *get_field_type(std::string name);
        std::vector<Type *> field_types() const;

        bool has_child(std::string name);
        Type *child_type(std::string name);

        std::string name() const;
        std::string mangled_name() const;

        RecordType *type() const {
            return m_type;
        }

        bool is_compatible(const Type *other) const;

        Record *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    protected:
        RecordType *m_type;
        std::vector<std::string> m_field_names;
    };

    class Tuple : public Record {
    public:
        Tuple(TupleType *type, std::vector<Type *> field_types);

        void accept(Visitor *visitor);
    };

    class Method : public Type {
    public:
        Method(MethodType *type, Type *return_type);
        Method(MethodType *type, std::vector<Type *> parameter_types, Type *return_type);
        Method(MethodType *type, Type *parameter1_type, Type *return_type);
        Method(MethodType *type, Type *parameter1_type, Type *parameter2_type, Type *return_type);
        Method(MethodType *type, Type *parameter1_type, Type *parameter2_type, Type *parameter3_type, Type *return_type);

        std::string name() const;
        std::string mangled_name() const;

        TypeType *type() const {
            return m_type;
        }

        std::vector<Type *> parameter_types() const;
        int parameter_index(std::string name) const;
        Type *return_type() const;

        template<typename T> std::vector<T> ordered_arguments(std::vector<T> positional_arguments, std::map<std::string, T> keyword_arguments, bool *valid = nullptr);
        std::vector<ast::Node *> ordered_arguments(ast::Call *call, bool *valid = nullptr);
        std::vector<Type *> ordered_argument_types(ast::Call *call, bool *valid = nullptr);

        bool could_be_called_with(std::vector<Type *> positional_arguments, std::map<std::string, Type *> keyword_arguments);

        void add_generic_specialisation(std::map<typesystem::ParameterType *, typesystem::Type *> specialisation);
        std::vector<std::map<typesystem::ParameterType *, typesystem::Type *> > generic_specialisations();
        size_t no_generic_specialisation() const;
        void add_empty_specialisation();

        void set_parameter_inout(Type *type, bool inout);
        bool is_parameter_inout(Type *type);

        void set_parameter_name(int index, std::string name);

        Method *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        MethodType *m_type;
        std::map<Type *, bool> m_inouts;
        std::map<std::string, int> m_names;
        std::vector<std::map<typesystem::ParameterType *, typesystem::Type *> > m_specialisations;
    };

    class Function : public Type {
    public:
        explicit Function(FunctionType *type);

        std::string name() const;
        std::string mangled_name() const;

        FunctionType *type() const {
            return m_type;
        }

        void add_method(Method *method);
        Method *find_method(ast::Node *node, std::vector<Type *> positional_arguments, std::map<std::string, Type *> keyword_arguments) const;
        Method *find_method(ast::Call *call) const;
        Method *get_method(int index) const;
        int no_methods() const;
        std::vector<Method *> methods() const;
        int index_of(Method *method) const;

        void set_llvm_index(Method *method, int index);
        int get_llvm_index(Method *method);

        Function *with_parameters(std::vector<Type *> parameters);

        void accept(Visitor *visitor);

    private:
        FunctionType *m_type;
        std::map<Method *, int> m_llvm_index;
    };

}
