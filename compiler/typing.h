//
// Created by Thomas Leese on 12/01/2017.
//

#ifndef ACORN_TYPING_H
#define ACORN_TYPING_H

#include <vector>

#include "ast.h"
#include "symboltable.h"

namespace acorn {

    namespace diagnostics {
        class Reporter;
    }

    namespace types {
        class Type;
        class TypeType;
        class Parameter;
    }

    namespace typing {

        class Inferrer : public ast::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower {

        public:
            explicit Inferrer(symboltable::Namespace *root_namespace);
            ~Inferrer();

        private:
            types::TypeType *find_type_constructor(ast::Node *node, std::string name);

            types::TypeType *find_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
            types::TypeType *find_type(ast::Node *node, std::string name);
            types::TypeType *find_type(ast::Name *name);

            types::Type *instance_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
            types::Type *instance_type(ast::Node *node, std::string name);
            types::Type *instance_type(ast::Name *name);

            types::Type *builtin_type_from_name(ast::Name *node);

            bool infer_call_type_parameters(ast::Call *call, std::vector<types::Type *> parameter_types,
                                            std::vector<types::Type *> argument_types);

            types::Type *replace_type_parameters(types::Type *type,
                                                 std::map<types::ParameterType *, types::Type *> replacements);

        public:
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

        private:
            std::vector<ast::Def *> m_functionStack;
            bool m_in_if;
            bool m_as_type;

        };

        class Checker : public ast::Visitor, public diagnostics::Reporter, public symboltable::ScopeFollower {

        public:
            explicit Checker(symboltable::Namespace *root_namespace);
            ~Checker();

        private:
            void check_types(ast::Expression *lhs, ast::Expression *rhs);
            void check_not_null(ast::Expression *expression);

        public:
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

#endif //ACORN_TYPING_H
