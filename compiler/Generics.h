//
// Created by Thomas Leese on 25/04/2016.
//

#ifndef JET_GENERICS_H
#define JET_GENERICS_H

#include "AbstractSyntaxTree.h"

namespace Generics {

    class Duplicator : public AST::Visitor {

    public:
        Duplicator();

        void visit(AST::CodeBlock *block);

        void visit(AST::Identifier *expression);
        void visit(AST::Type *type);
        void visit(AST::BooleanLiteral *boolean);
        void visit(AST::IntegerLiteral *expression);
        void visit(AST::FloatLiteral *expression);
        void visit(AST::ImaginaryLiteral *imaginary);
        void visit(AST::StringLiteral *expression);
        void visit(AST::SequenceLiteral *sequence);
        void visit(AST::MappingLiteral *mapping);
        void visit(AST::Argument *argument);
        void visit(AST::Call *expression);
        void visit(AST::CCall *expression);
        void visit(AST::Assignment *expression);
        void visit(AST::Selector *expression);
        void visit(AST::Index *expression);
        void visit(AST::Comma *expression);
        void visit(AST::While *expression);
        void visit(AST::For *expression);
        void visit(AST::If *expression);
        void visit(AST::Return *expression);
        void visit(AST::Spawn *expression);

        void visit(AST::Parameter *parameter);

        void visit(AST::VariableDefinition *definition);
        void visit(AST::FunctionDefinition *definition);
        void visit(AST::TypeDefinition *definition);

        void visit(AST::DefinitionStatement *statement);
        void visit(AST::ExpressionStatement *statement);
        void visit(AST::ImportStatement *statement);

        void visit(AST::SourceFile *module);

    private:
        bool m_collecting;

        std::vector<AST::TypeDefinition *> m_generic_types;
        std::vector<AST::FunctionDefinition *> m_generic_functions;

        std::vector<AST::TypeDefinition

    };

}

#endif //JET_GENERICS_H
