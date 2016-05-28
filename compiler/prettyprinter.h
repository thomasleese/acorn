//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef ACORN_PRETTYPRINTER_H
#define ACORN_PRETTYPRINTER_H

#include <sstream>
#include <string>

#include "ast/visitor.h"

namespace acorn {

    class PrettyPrinter : public ast::Visitor {

    public:
        explicit PrettyPrinter();

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
        void visit(ast::CCall *expression);
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
        std::string indentation();

    public:
        std::string str();
        void print();

    private:
        int indent;
        std::stringstream ss;

    };

}

#endif // ACORN_PRETTYPRINTER_H
