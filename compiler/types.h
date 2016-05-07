//
// Created by Thomas Leese on 18/03/2016.
//

#ifndef JET_TYPES_H
#define JET_TYPES_H

#include <set>
#include <string>
#include <vector>
#include <map>

namespace jet {

    namespace ast {
        struct Node;
        struct Argument;
    }

    namespace types {

        class Visitor;

        class Type {

        public:
            virtual ~Type();

            virtual std::string name() const = 0;

            virtual bool isCompatible(const Type *other) const;

            virtual std::string mangled_name() const = 0;

            virtual void accept(Visitor *visitor) = 0;
        };

        class Parameter : public Type {

        public:
            std::string name() const;
            std::string mangled_name() const;

            void accept(Visitor *visitor);

        };

        // type constructors
        class Constructor : public Type {
        public:
            Type *create(ast::Node *node);

            virtual Type *create(ast::Node *node, std::vector<Type *> parameters) = 0;

            std::string mangled_name() const;
        };

        class AnyConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class VoidConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class BooleanConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class IntegerConstructor : public Constructor {
        public:
            IntegerConstructor(unsigned int size);

            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsignedIntegerConstructor : public Constructor {
        public:
            UnsignedIntegerConstructor(unsigned int size);

            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class FloatConstructor : public Constructor {
        public:
            FloatConstructor(int size);

            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            int m_size;
        };

        class UnsafePointerConstructor : public Constructor {

        public:
            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        };

        class FunctionConstructor : public Constructor {
        public:
            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);
        };

        class RecordConstructor : public Constructor {

        public:
            RecordConstructor();
            RecordConstructor(std::vector<Parameter *> input_parameters, std::vector<std::string> field_names,
                              std::vector<Constructor *> field_types,
                              std::vector<std::vector<Type *> > field_parameters);

            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

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

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        };

        class AliasConstructor : public Constructor {

        public:
            explicit AliasConstructor(ast::Node *node, Constructor *constructor,
                                      std::vector<Parameter *> input_arameters, std::vector<Type *> outputParameters);

            std::string name() const;

            Type *create(ast::Node *node, std::vector<Type *> parameters);

            void accept(Visitor *visitor);

        private:
            Constructor *m_constructor;
            std::map<int, int> m_parameterMapping;
            std::vector<Type *> m_knownTypes;

        };

        // concrete types
        class Any : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            void accept(Visitor *visitor);
        };

        class Void : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            void accept(Visitor *visitor);
        };

        class Boolean : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            void accept(Visitor *visitor);
        };

        class Integer : public Type {
        public:
            explicit Integer(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            unsigned int size() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsignedInteger : public Type {
        public:
            explicit UnsignedInteger(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            unsigned int size() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class Float : public Type {
        public:
            explicit Float(unsigned int size);

            std::string name() const;
            std::string mangled_name() const;

            unsigned int size() const;

            void accept(Visitor *visitor);

        private:
            unsigned int m_size;
        };

        class UnsafePointer : public Type {
        public:
            explicit UnsafePointer(Type *element_type);

            std::string name() const;
            std::string mangled_name() const;

            Type *element_type() const;

            void accept(Visitor *visitor);

        private:
            Type *m_element_type;
        };

        class Record : public Type {
        public:
            Record(std::vector<std::string> field_names, std::vector<Type *> field_types);

            bool has_field(std::string name);
            long get_field_index(std::string name);
            Type *get_field_type(std::string name);
            std::vector<Type *> field_types() const;

            std::string name() const;
            std::string mangled_name() const;

            void accept(Visitor *visitor);

        protected:
            std::vector<std::string> m_field_names;
            std::vector<Type *> m_field_types;
        };

        class Tuple : public Record {
        public:
            Tuple(std::vector<Type *> field_types);

            void accept(Visitor *visitor);
        };

        class Method : public Type {
        public:
            Method(std::vector<Type *> parameter_types, Type *return_type,
                   std::vector<std::string> official_parameter_order);
            Method(Type *return_type);
            Method(std::string parameter1_name, Type *parameter1_type, Type *return_type);
            Method(std::string parameter1_name, Type *parameter1_type, std::string parameter2_name,
                   Type *parameter2_type, Type *return_type);

            std::string name() const;
            std::string mangled_name() const;

            std::vector<Type *> parameter_types() const;
            Type *return_type() const;
            long get_parameter_position(std::string name) const;
            std::string get_parameter_name(long position) const;
            bool could_be_called_with(std::vector<ast::Argument *> arguments);

            void accept(Visitor *visitor);

        private:
            std::vector<Type *> m_parameter_types;
            Type *m_return_type;
            std::vector<std::string> m_official_parameter_order;
        };

        class Function : public Type {
        public:
            std::string name() const;
            std::string mangled_name() const;

            void add_method(Method *method);
            Method *find_method(ast::Node *node, std::vector<ast::Argument *> arguments) const;
            Method *get_method(int index) const;
            int no_methods() const;

            void accept(Visitor *visitor);

        private:
            std::vector<Method *> m_methods;
        };

        class Union : public Type {
        public:
            Union(Type *type1, Type *type2);

            Union(ast::Node *node, std::set<Type *> types);

            std::string name() const;
            std::string mangled_name() const;

            std::set<Type *> types() const;
            bool isCompatible(const Type *other) const;

            void accept(Visitor *visitor);

        private:
            std::set<Type *> m_types;
        };

        class Visitor {
        public:
            virtual ~Visitor();

            virtual void visit(Parameter *type) = 0;
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
            virtual void visit(AliasConstructor *type) = 0;
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

#endif // JET_TYPES_H