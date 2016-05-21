//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef ACORN_TYPES_H
#define ACORN_TYPES_H

#include <set>
#include <string>
#include <vector>
#include <map>

namespace acorn {

    namespace ast {
        struct Node;
        struct Argument;
    }

    namespace compiler {
        class Pass;
    }

    namespace types {

        class Visitor;

        class Type {

        public:
            virtual ~Type();

            virtual std::string name() const = 0;

            virtual bool isCompatible(const Type *other) const;

            virtual std::string mangled_name() const = 0;

            virtual Type *clone() const = 0;

            Type *get_parameter(int i) const;
            void set_parameter(int i, Type *type);
            std::vector<Type *> parameters() const;

            virtual void accept(Visitor *visitor) = 0;

        protected:
            std::vector<Type *> m_parameters;
        };

        class Parameter;

        // type constructors
        class Constructor : public Type {
        public:
            Type *create(compiler::Pass *pass, ast::Node *node);
            virtual Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters) = 0;

            std::string mangled_name() const;

            virtual Constructor *clone() const = 0;
        };

        class ParameterConstructor : public Constructor {
        public:
            std::string name() const;

            bool isCompatible(const Type *other) const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            ParameterConstructor *clone() const;

            void accept(Visitor *visitor);
        };

        class AnyConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            AnyConstructor *clone() const;

            void accept(Visitor *visitor);
        };

        class VoidConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            VoidConstructor *clone() const;

            void accept(Visitor *visitor);
        };

        class BooleanConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            BooleanConstructor *clone() const;

            void accept(Visitor *visitor);
        };

        class IntegerConstructor : public Constructor {
        public:
            IntegerConstructor(unsigned int size);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            IntegerConstructor *clone() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsignedIntegerConstructor : public Constructor {
        public:
            UnsignedIntegerConstructor(unsigned int size);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            UnsignedIntegerConstructor *clone() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class FloatConstructor : public Constructor {
        public:
            FloatConstructor(int size);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            FloatConstructor *clone() const;

            void accept(Visitor *visitor);

        private:
            int m_size;
        };

        class UnsafePointerConstructor : public Constructor {

        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            UnsafePointerConstructor *clone() const;

            void accept(Visitor *visitor);

        };

        class FunctionConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            FunctionConstructor *clone() const;

            void accept(Visitor *visitor);
        };

        class RecordConstructor : public Constructor {

        public:
            RecordConstructor();
            RecordConstructor(std::vector<Parameter *> input_parameters, std::vector<std::string> field_names,
                              std::vector<Constructor *> field_types,
                              std::vector<std::vector<Type *> > field_parameters);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            RecordConstructor *clone() const;

            void accept(Visitor *visitor);

        private:
            std::vector<Parameter *> m_input_parameters;
            std::vector<std::string> m_field_names;
            std::vector<Constructor *> m_field_types;
            std::vector<std::vector<Type *> > m_field_parameters;

        };

        class UnionConstructor : public Constructor {

        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            UnionConstructor *clone() const;

            void accept(Visitor *visitor);

        };

        class TupleConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            TupleConstructor *clone() const;

            void accept(Visitor *visitor);
        };

        class AliasConstructor : public Constructor {

        public:
            explicit AliasConstructor(Constructor *constructor, std::vector<Parameter *> input_parameters,
                                      std::vector<Type *> output_parameters);

            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            AliasConstructor *clone() const;

            void accept(Visitor *visitor);

        private:
            Constructor *m_constructor;
            std::vector<Parameter *> m_input_parameters;
            std::vector<Type *> m_output_parameters;
            std::map<int, int> m_parameterMapping;
            std::vector<Type *> m_knownTypes;

        };

        class TypeDescriptionConstructor : public Constructor {

        public:
            std::string name() const;

            Type *create(compiler::Pass *pass, ast::Node *node, std::vector<Type *> parameters);

            TypeDescriptionConstructor *clone() const;

            void accept(Visitor *visitor);

        };

        class ConstructedType : public Type {
        public:
            virtual Constructor *constructor() const = 0;
        };

        // constructed types
        class Parameter : public ConstructedType {

        public:
            Parameter(ParameterConstructor *constructor);

            std::string name() const;
            std::string mangled_name() const;

            ParameterConstructor *constructor() const;

            bool isCompatible(const Type *other) const;

            Parameter *clone() const;

            void accept(Visitor *visitor);

        private:
            ParameterConstructor *m_constructor;

        };

        class Any : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            Any *clone() const;

            void accept(Visitor *visitor);
        };

        class Void : public ConstructedType {
        public:
            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            Void *clone() const;

            void accept(Visitor *visitor);
        };

        class Boolean : public ConstructedType {
        public:
            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            Boolean *clone() const;

            void accept(Visitor *visitor);
        };

        class Integer : public ConstructedType {
        public:
            explicit Integer(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            unsigned int size() const;

            Integer *clone() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsignedInteger : public ConstructedType {
        public:
            explicit UnsignedInteger(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            unsigned int size() const;

            UnsignedInteger *clone() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class Float : public ConstructedType {
        public:
            explicit Float(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            unsigned int size() const;

            Float *clone() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsafePointer : public ConstructedType {
        public:
            explicit UnsafePointer(Type *element_type);

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            Type *element_type() const;

            bool isCompatible(const Type *other) const;

            UnsafePointer *clone() const;

            void accept(Visitor *visitor);
        };

        class Record : public ConstructedType {
        public:
            Record(std::vector<std::string> field_names, std::vector<Type *> field_types);

            bool has_field(std::string name);
            long get_field_index(std::string name);
            Type *get_field_type(std::string name);
            std::vector<Type *> field_types() const;

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            bool isCompatible(const Type *other) const;

            Record *clone() const;

            void accept(Visitor *visitor);

        protected:
            std::vector<std::string> m_field_names;
        };

        class Tuple : public Record {
        public:
            Tuple(std::vector<Type *> field_types);

            void accept(Visitor *visitor);
        };

        class Method : public Type {
        public:
            Method(std::vector<Type *> parameter_types, Type *return_type);
            Method(Type *return_type);
            Method(Type *parameter1_type, Type *return_type);
            Method(Type *parameter1_type, Type *parameter2_type, Type *return_type);
            Method(Type *parameter1_type, Type *parameter2_type, Type *parameter3_type, Type *return_type);

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            void set_is_generic(bool is_generic);
            bool is_generic() const;

            std::vector<Type *> parameter_types() const;
            Type *return_type() const;
            bool could_be_called_with(std::vector<Type *> arguments);

            void set_parameter_inout(Type *type, bool inout);
            bool is_parameter_inout(Type *type);

            Method *clone() const;

            void accept(Visitor *visitor);

        private:
            std::map<Type *, bool> m_inouts;
            bool m_is_generic;
        };

        class Function : public ConstructedType {
        public:
            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            void add_method(Method *method);
            Method *find_method(ast::Node *node, std::vector<Type *> arguments) const;
            Method *get_method(int index) const;
            int no_methods() const;

            Function *clone() const;

            void accept(Visitor *visitor);
        };

        class Union : public ConstructedType {
        public:
            Union(Type *type1, Type *type2);
            Union(std::vector<Type *> types);

            std::string name() const;
            std::string mangled_name() const;

            Constructor *constructor() const;

            std::vector<Type *> types() const;
            uint8_t type_index(const Type *type, bool *exists) const;

            bool isCompatible(const Type *other) const;

            Union *clone() const;

            void accept(Visitor *visitor);
        };

        class Visitor {
        public:
            virtual ~Visitor();

            virtual void visit(ParameterConstructor *type) = 0;
            virtual void visit(AnyConstructor *type) = 0;
            virtual void visit(VoidConstructor *type) = 0;
            virtual void visit(BooleanConstructor *type) = 0;
            virtual void visit(IntegerConstructor *type) = 0;
            virtual void visit(UnsignedIntegerConstructor *type) = 0;
            virtual void visit(FloatConstructor *type) = 0;
            virtual void visit(UnsafePointerConstructor *type) = 0;
            virtual void visit(FunctionConstructor *type) = 0;
            virtual void visit(RecordConstructor *type) = 0;
            virtual void visit(UnionConstructor *type) = 0;
            virtual void visit(TupleConstructor *type) = 0;
            virtual void visit(AliasConstructor *type) = 0;
            virtual void visit(TypeDescriptionConstructor *type) = 0;
            virtual void visit(Parameter *type) = 0;
            virtual void visit(Any *type) = 0;
            virtual void visit(Void *type) = 0;
            virtual void visit(Boolean *type) = 0;
            virtual void visit(Integer *type) = 0;
            virtual void visit(UnsignedInteger *type) = 0;
            virtual void visit(Float *type) = 0;
            virtual void visit(UnsafePointer *type) = 0;
            virtual void visit(Record *type) = 0;
            virtual void visit(Tuple *type) = 0;
            virtual void visit(Method *type) = 0;
            virtual void visit(Function *type) = 0;
            virtual void visit(Union *type) = 0;
        };

    };

}

#endif // ACORN_TYPES_H
