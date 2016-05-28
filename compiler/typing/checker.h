//
// Created by Thomas Leese on 22/05/2016.
//

#ifndef ACORN_TYPING_CHECKER_H
#define ACORN_TYPING_CHECKER_H

#include <vector>

#include "../ast/visitor.h"
#include "../pass.h"

namespace acorn {

    namespace ast {
        struct Node;
    }

    namespace symboltable {
        class Namespace;
    }

    namespace types {
        class Type;
        class TypeType;
        class Parameter;
    }

    namespace typing {

        class Checker : public compiler::Pass, public ast::Visitor {

        public:
            explicit Checker(symboltable::Namespace *rootNamespace);
            ~Checker();

        private:
            void check_types(ast::Node *lhs, ast::Node *rhs);
            void check_not_null(ast::Node *node);

        public:
            void visit(ast::CodeBlock *block);
            void visit(ast::Identifier *expression);
            void visit(ast::VariableDeclaration *node);
            void visit(ast::IntegerLiteral *expression);
            void visit(ast::FloatLiteral *expression);
            void visit(ast::ImaginaryLiteral *imaginary);
            void visit(ast::StringLiteral *expression);
            void visit(ast::SequenceLiteral *sequence);
            void visit(ast::MappingLiteral *mapping);
            void visit(ast::RecordLiteral *expression);
            void visit(ast::TupleLiteral *expression);
            void visit(ast::Call *expression);
            void visit(ast::CCall *ccall);
            void visit(ast::Cast *expression);
            void visit(ast::Assignment *expression);
            void visit(ast::Selector *expression);
            void visit(ast::While *expression);
            void visit(ast::If *expression);
            void visit(ast::Return *expression);
            void visit(ast::Spawn *expression);
            void visit(ast::Parameter *parameter);
            void visit(ast::VariableDefinition *definition);
            void visit(ast::FunctionDefinition *definition);
            void visit(ast::TypeDefinition *definition);
            void visit(ast::ProtocolDefinition *definition);
            void visit(ast::EnumDefinition *definition);
            void visit(ast::DefinitionStatement *statement);
            void visit(ast::ExpressionStatement *statement);
            void visit(ast::ImportStatement *statement);
            void visit(ast::SourceFile *module);

        private:
            symboltable::Namespace *m_namespace;

        };

    };

}

#endif // ACORN_TYPING_CHECKER_H
