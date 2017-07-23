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
        class ParameterType;
    }

    namespace typesystem {

        class TypeInferrer : public ast::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower {

        public:
            explicit TypeInferrer(symboltable::Namespace *scope);

        private:
            typesystem::TypeType *find_type_constructor(ast::Node *node, std::string name);

            typesystem::TypeType *find_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
            typesystem::TypeType *find_type(ast::Node *node, std::string name);
            typesystem::TypeType *find_type(ast::Name *name);

            typesystem::Type *instance_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
            typesystem::Type *instance_type(ast::Node *node, std::string name);
            typesystem::Type *instance_type(ast::Name *name);

            typesystem::Type *builtin_type_from_name(ast::Name *node);

            bool infer_call_type_parameters(ast::Call *call, std::vector<typesystem::Type *> parameter_types,
                                            std::vector<typesystem::Type *> argument_types);

            typesystem::Type *replace_type_parameters(typesystem::Type *type,
                                                      std::map<typesystem::ParameterType *, typesystem::Type *> replacements);

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

        private:
            std::vector<ast::Def *> m_function_stack;

        };

    }

}
