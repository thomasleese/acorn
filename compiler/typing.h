//
// Created by Thomas Leese on 12/01/2017.
//

#ifndef ACORN_TYPING_H
#define ACORN_TYPING_H

#include <vector>

#include "ast.h"

namespace acorn {

    namespace diagnostics {
        class Reporter;
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

        class Inferrer : public ast::Visitor, public diagnostics::Reporter {

        public:
            explicit Inferrer(symboltable::Namespace *root_namespace);
            ~Inferrer();

        private:
            types::TypeType *find_type_constructor(ast::Node *node, std::string name);

            types::TypeType *find_type(ast::Node *node, std::string name, std::vector<ast::Identifier *> parameters);
            types::TypeType *find_type(ast::Node *node, std::string name);
            types::TypeType *find_type(ast::Identifier *type);

            types::Type *instance_type(ast::Node *node, std::string name, std::vector<ast::Identifier *> parameters);
            types::Type *instance_type(ast::Node *node, std::string name);
            types::Type *instance_type(ast::Identifier *identifier);

            bool infer_call_type_parameters(ast::Call *call, std::vector<types::Type *> parameter_types,
                                            std::vector<types::Type *> argument_types);

            types::Type *replace_type_parameters(types::Type *type,
                                                 std::map<types::ParameterType *, types::Type *> replacements);

        public:
            void visit(ast::CodeBlock *block);
            void visit(ast::Identifier *identifier);
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
            void visit(ast::Switch *expression);
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
            std::vector<ast::FunctionDefinition *> m_functionStack;
            bool m_in_if;
            bool m_as_type;

        };

        class Checker : public ast::Visitor, public diagnostics::Reporter {

        public:
            explicit Checker(symboltable::Namespace *root_namespace);
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
            void visit(ast::Switch *expression);
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

    }

}

#endif //ACORN_TYPING_H
