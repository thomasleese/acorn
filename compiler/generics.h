//
// Created by Thomas Leese on 09/04/2017.
//

#pragma once

#include <sstream>
#include <string>

#include "ast.h"
#include "diagnostics.h"
#include "symboltable.h"

namespace acorn {

    namespace generics {

        class Reification : public ast::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower {

        public:
            Reification(symboltable::Namespace *scope);

            void visit(ast::Block *node);
            void visit(ast::Name *node);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::Int *node);
            void visit(ast::Float *node);
            void visit(ast::Complex *node);
            void visit(ast::String *node);
            void visit(ast::List *node);
            void visit(ast::Dictionary *node);
            void visit(ast::Tuple *node);
            void visit(ast::Call *node);
            void visit(ast::CCall *node);
            void visit(ast::Cast *node);
            void visit(ast::Assignment *node);
            void visit(ast::Selector *node);
            void visit(ast::While *node);
            void visit(ast::If *node);
            void visit(ast::Return *node);
            void visit(ast::Spawn *node);
            void visit(ast::Switch *node);
            void visit(ast::Parameter *node);
            void visit(ast::Let *node);
            void visit(ast::Def *node);
            void visit(ast::Type *node);
            void visit(ast::Module *node);
            void visit(ast::Import *node);
            void visit(ast::SourceFile *node);

        };

    }

}
