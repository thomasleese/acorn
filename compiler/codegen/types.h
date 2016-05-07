//
// Created by Thomas Leese on 27/04/2016.
//

#ifndef JET_CODEGEN_TYPES_H
#define JET_CODEGEN_TYPES_H

#include "../types.h"

namespace jet {

    namespace ast {
        struct Node;
    }

    namespace codegen {

        class TypeGenerator : public types::Visitor {
        public:
            llvm::Type *take_type(ast::Node *node);
            llvm::Constant *take_initialiser(ast::Node *node);

            void visit(types::Parameter *type);
            void visit(types::AnyConstructor *type);
            void visit(types::VoidConstructor *type);
            void visit(types::BooleanConstructor *type);
            void visit(types::IntegerConstructor *type);
            void visit(types::UnsignedIntegerConstructor *type);
            void visit(types::FloatConstructor *type);
            void visit(types::UnsafePointerConstructor *type);
            void visit(types::FunctionConstructor *type);
            void visit(types::RecordConstructor *type);
            void visit(types::UnionConstructor *type);
            void visit(types::AliasConstructor *type);
            void visit(types::Any *type);
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
            void visit(types::Union *type);

        private:
            std::vector<llvm::Type *> m_type_stack;
            std::vector<llvm::Constant *> m_initialiser_stack;
        };

    }

}

#endif // JET_CODEGEN_TYPES_H
