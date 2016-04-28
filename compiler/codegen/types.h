//
// Created by Thomas Leese on 27/04/2016.
//

#ifndef JET_CODEGEN_TYPES_H
#define JET_CODEGEN_TYPES_H

#include "../Types.h"

namespace AST {
    struct Node;
}

namespace jet {

    namespace codegen {

        class TypeGenerator : public Types::Visitor {
        public:
            llvm::Type *take_result(AST::Node *node);

            void visit(Types::Constructor *type);
            void visit(Types::AnyConstructor *type);
            void visit(Types::VoidConstructor *type);
            void visit(Types::BooleanConstructor *type);
            void visit(Types::IntegerConstructor *type);
            void visit(Types::UnsignedIntegerConstructor *type);
            void visit(Types::FloatConstructor *type);
            void visit(Types::UnsafePointerConstructor *type);
            void visit(Types::FunctionConstructor *type);
            void visit(Types::RecordConstructor *type);
            void visit(Types::UnionConstructor *type);
            void visit(Types::AliasConstructor *type);
            void visit(Types::Any *type);
            void visit(Types::Void *type);
            void visit(Types::Boolean *type);
            void visit(Types::Integer *type);
            void visit(Types::UnsignedInteger *type);
            void visit(Types::Float *type);
            void visit(Types::UnsafePointer *type);
            void visit(Types::Record *type);
            void visit(Types::Tuple *type);
            void visit(Types::Method *type);
            void visit(Types::Function *type);
            void visit(Types::Union *type);

        private:
            std::vector<llvm::Type *> m_stack;
        };

        class TypeInitialiserGenerator : public Types::Visitor {
        public:
            llvm::Constant *take_result(AST::Node *node);

            void visit(Types::Constructor *type);
            void visit(Types::AnyConstructor *type);
            void visit(Types::VoidConstructor *type);
            void visit(Types::BooleanConstructor *type);
            void visit(Types::IntegerConstructor *type);
            void visit(Types::UnsignedIntegerConstructor *type);
            void visit(Types::FloatConstructor *type);
            void visit(Types::UnsafePointerConstructor *type);
            void visit(Types::FunctionConstructor *type);
            void visit(Types::RecordConstructor *type);
            void visit(Types::UnionConstructor *type);
            void visit(Types::AliasConstructor *type);
            void visit(Types::Any *type);
            void visit(Types::Void *type);
            void visit(Types::Boolean *type);
            void visit(Types::Integer *type);
            void visit(Types::UnsignedInteger *type);
            void visit(Types::Float *type);
            void visit(Types::UnsafePointer *type);
            void visit(Types::Record *type);
            void visit(Types::Tuple *type);
            void visit(Types::Method *type);
            void visit(Types::Function *type);
            void visit(Types::Union *type);

        private:
            std::vector<llvm::Constant *> m_stack;
        };

    }

}

#endif // JET_CODEGEN_TYPES_H
