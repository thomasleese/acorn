//
// Created by Thomas Leese on 21/03/2016.
//

#ifndef JET_TYPING_H
#define JET_TYPING_H

#include <vector>

#include "ast/visitor.h"

namespace jet {

    namespace ast {
        struct Node;
    }

    namespace symboltable {
        class Namespace;
    }

    namespace types {
        class Type;
        class Constructor;
        class Parameter;
    }

    namespace typing {

        class Inferrer : public ast::Visitor {

        public:
            explicit Inferrer(symboltable::Namespace *rootNamespace);
            ~Inferrer();

        private:
            types::Constructor *find_type_constructor(ast::Node *node, std::string name);
            types::Type *find_type(ast::Node *node, std::string name, std::vector<ast::Identifier *> parameters);
            types::Type *find_type(ast::Node *node, std::string name);
            types::Type *find_type(ast::Identifier *type);
            types::Type *instance_type(ast::Identifier *identifier);

        public:
            void visit(ast::CodeBlock *block);
            void visit(ast::Identifier *identifier);
            void visit(ast::BooleanLiteral *boolean);
            void visit(ast::IntegerLiteral *expression);
            void visit(ast::FloatLiteral *expression);
            void visit(ast::ImaginaryLiteral *imaginary);
            void visit(ast::StringLiteral *expression);
            void visit(ast::SequenceLiteral *sequence);
            void visit(ast::MappingLiteral *mapping);
            void visit(ast::RecordLiteral *expression);
            void visit(ast::Argument *argument);
            void visit(ast::Call *expression);
            void visit(ast::CCall *ccall);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::Index *expression);
            void visit(ast::Comma *expression);
            void visit(ast::While *expression);
            void visit(ast::For *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Sizeof *expression);
            void visit(ast::Strideof *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::VariableDefinition *definition);
            void visit(ast::FunctionDefinition *definition);
            void visit(ast::TypeDefinition *definition);
            void visit(ast::DefinitionStatement *statement);
            void visit(ast::ExpressionStatement *statement);
            void visit(ast::ImportStatement *statement);
            void visit(ast::SourceFile *module);

        private:
            symboltable::Namespace *m_namespace;
            std::vector<ast::FunctionDefinition *> m_functionStack;

        };

        class Checker : public ast::Visitor {

        public:
            explicit Checker(symboltable::Namespace *rootNamespace);
            ~Checker();

        private:
            void check_types(ast::Node *lhs, ast::Node *rhs);
            void check_not_null(ast::Node *node);

        public:
            void visit(ast::CodeBlock *block);
            void visit(ast::Identifier *expression);
            void visit(ast::BooleanLiteral *boolean);
            void visit(ast::IntegerLiteral *expression);
            void visit(ast::FloatLiteral *expression);
            void visit(ast::ImaginaryLiteral *imaginary);
            void visit(ast::StringLiteral *expression);
            void visit(ast::SequenceLiteral *sequence);
            void visit(ast::MappingLiteral *mapping);
            void visit(ast::RecordLiteral *expression);
            void visit(ast::Argument *argument);
            void visit(ast::Call *expression);
            void visit(ast::CCall *ccall);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::Index *expression);
            void visit(ast::Comma *expression);
            void visit(ast::While *expression);
            void visit(ast::For *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Sizeof *expression);
            void visit(ast::Strideof *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::VariableDefinition *definition);
            void visit(ast::FunctionDefinition *definition);
            void visit(ast::TypeDefinition *definition);
            void visit(ast::DefinitionStatement *statement);
            void visit(ast::ExpressionStatement *statement);
            void visit(ast::ImportStatement *statement);
            void visit(ast::SourceFile *module);

        private:
            symboltable::Namespace *m_namespace;

        };

    };

}

#endif //JET_TYPING_H
