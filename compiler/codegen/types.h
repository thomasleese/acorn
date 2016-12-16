//
// Created by Thomas Leese on 27/04/2016.
//

#ifndef ACORN_CODEGEN_TYPES_H
#define ACORN_CODEGEN_TYPES_H

#include "../typing/types.h"

namespace llvm {
    class Constant;
    class Type;
}

namespace acorn {

    namespace ast {
        struct Node;
    }

    namespace codegen {

        class TypeGenerator : public types::Visitor {
        public:
            explicit TypeGenerator(diagnostics::Reporter *diagnostics, llvm::LLVMContext &context);

            llvm::Type *take_type(ast::Node *node);
            llvm::Constant *take_initialiser(ast::Node *node);

            void push_type_parameter(types::ParameterType *key, types::Type *value);
            void pop_type_parameter(types::ParameterType *key);
            types::Type *get_type_parameter(types::ParameterType *key);
            types::Type *get_type_parameter(types::Parameter *key);

            void visit_constructor(types::TypeType *type);

            void visit(types::ParameterType *type);
            void visit(types::VoidType *type);
            void visit(types::BooleanType *type);
            void visit(types::IntegerType *type);
            void visit(types::UnsignedIntegerType *type);
            void visit(types::FloatType *type);
            void visit(types::UnsafePointerType *type);
            void visit(types::FunctionType *type);
            void visit(types::RecordType *type);
            void visit(types::EnumType *type);
            void visit(types::TupleType *type);
            void visit(types::AliasType *type);
            void visit(types::TypeDescriptionType *type);
            void visit(types::ProtocolType *type);

            void visit(types::Parameter *type);
            void visit(types::Void *type);
            void visit(types::Boolean *type);
            void visit(types::Integer *type);
            void visit(types::UnsignedInteger *type);
            void visit(types::Float *type);
            void visit(types::UnsafePointer *type);
            void visit(types::Record *type);
            void visit(types::Tuple *type);
            void visit(types::Method *type);
            void visit(types::Function *type);
            void visit(types::Enum *type);
            void visit(types::Protocol *type);

        private:
            llvm::LLVMContext &m_context;
            std::vector<llvm::Type *> m_type_stack;
            std::vector<llvm::Constant *> m_initialiser_stack;

            std::map<types::ParameterType *, types::Type *> m_type_parameters;

            diagnostics::Reporter *m_diagnostics;
        };

    }

}

#endif // ACORN_CODEGEN_TYPES_H
