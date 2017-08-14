//
// Created by Thomas Leese on 12/01/2017.
//

#pragma once

#include <vector>

#include "../ast/visitor.h"
#include "../symboltable.h"

namespace acorn {

    namespace diagnostics {
        class Reporter;
    }

    namespace typesystem {
        class Type;
        class TypeType;
    }

}

namespace acorn::typesystem {

    class TypeChecker : public ast::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower {

    public:
        explicit TypeChecker(symboltable::Namespace *scope);

    private:
        void check_types(ast::Expression *lhs, ast::Expression *rhs);
        void check_not_null(ast::Expression *expression);

    public:
        void visit(ast::Block *node) override;
        void visit(ast::Name *node) override;
        void visit(ast::VariableDeclaration *node) override;
        void visit(ast::Int *node) override;
        void visit(ast::Float *node) override;
        void visit(ast::Complex *node) override;
        void visit(ast::String *node) override;
        void visit(ast::List *node) override;
        void visit(ast::Dictionary *node) override;
        void visit(ast::Tuple *node) override;
        void visit(ast::Call *node) override;
        void visit(ast::CCall *node) override;
        void visit(ast::Cast *node) override;
        void visit(ast::Assignment *node) override;
        void visit(ast::Selector *node) override;
        void visit(ast::While *node) override;
        void visit(ast::If *node) override;
        void visit(ast::Return *node) override;
        void visit(ast::Spawn *node) override;
        void visit(ast::Switch *node) override;
        void visit(ast::Parameter *node) override;
        void visit(ast::Let *node) override;
        void visit(ast::Def *node) override;
        void visit(ast::Type *node) override;
        void visit(ast::Module *node) override;
        void visit(ast::Import *node) override;
        void visit(ast::SourceFile *node) override;

    };

}
