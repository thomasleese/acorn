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
            types::TypeType *find_type(ast::Name *type);

            types::Type *instance_type(ast::Node *node, std::string name, std::vector<ast::Name *> parameters);
            types::Type *instance_type(ast::Node *node, std::string name);
            types::Type *instance_type(ast::Name *identifier);

            types::Type *builtin_type_from_name(ast::Name *node);

            bool infer_call_type_parameters(ast::Call *call, std::vector<types::Type *> parameter_types,
                                            std::vector<types::Type *> argument_types);

            types::Type *replace_type_parameters(types::Type *type,
                                                 std::map<types::ParameterType *, types::Type *> replacements);

        public:
            void visit(ast::Block *block);
            void visit(ast::Name *identifier);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::Int *expression);
            void visit(ast::Float *expression);
            void visit(ast::Complex *imaginary);
            void visit(ast::String *expression);
            void visit(ast::List *sequence);
            void visit(ast::Dictionary *mapping);
            void visit(ast::Tuple *expression);
            void visit(ast::Call *expression);
            void visit(ast::CCall *ccall);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::While *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Switch *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::Let *definition);
            void visit(ast::Def *definition);
            void visit(ast::Type *definition);
            void visit(ast::Module *module);
            void visit(ast::Import *statement);
            void visit(ast::SourceFile *module);

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
            void check_not_null(ast::Expression &expression);

        public:
            void visit(ast::Block *block);
            void visit(ast::Name *expression);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::Int *expression);
            void visit(ast::Float *expression);
            void visit(ast::Complex *imaginary);
            void visit(ast::String *expression);
            void visit(ast::List *sequence);
            void visit(ast::Dictionary *mapping);
            void visit(ast::Tuple *expression);
            void visit(ast::Call *expression);
            void visit(ast::CCall *ccall);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::While *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Switch *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::Let *definition);
            void visit(ast::Def *definition);
            void visit(ast::Type *definition);
            void visit(ast::Module *module);
            void visit(ast::Import *statement);
            void visit(ast::SourceFile *module);

        };

    }

}

#endif //ACORN_TYPING_H
