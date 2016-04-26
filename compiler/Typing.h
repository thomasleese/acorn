//
// Created by Thomas Leese on 21/03/2016.
//

#ifndef JET_TYPING_H
#define JET_TYPING_H

#include "AbstractSyntaxTree.h"

namespace SymbolTable {
    class Namespace;
}

namespace Types {
    class Type;
    class Constructor;
    class Parameter;
}

namespace Typing {

    class Inferrer : public AST::Visitor {

    public:
        explicit Inferrer(SymbolTable::Namespace *rootNamespace);
        ~Inferrer();

    private:
        Types::Constructor *find_type_constructor(AST::Node *node, std::string name);

        Types::Type *find_type(AST::Node *node, std::string name, std::vector<AST::Identifier *> parameters);
        Types::Type *find_type(AST::Node *node, std::string name);
        Types::Type *find_type(AST::Identifier *type);

        Types::Type *instance_type(AST::Identifier *identifier);

    public:
        void visit(AST::CodeBlock *block);

        void visit(AST::Identifier *identifier);
        void visit(AST::BooleanLiteral *boolean);
        void visit(AST::IntegerLiteral *expression);
        void visit(AST::FloatLiteral *expression);
        void visit(AST::ImaginaryLiteral *imaginary);
        void visit(AST::StringLiteral *expression);
        void visit(AST::SequenceLiteral *sequence);
        void visit(AST::MappingLiteral *mapping);
        void visit(AST::Argument *argument);
        void visit(AST::Call *expression);
        void visit(AST::CCall *ccall);
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
        SymbolTable::Namespace *m_namespace;
        std::vector<AST::FunctionDefinition *> m_functionStack;

    };

    class Checker : public AST::Visitor {

    public:
        explicit Checker(SymbolTable::Namespace *rootNamespace);
        ~Checker();

    private:
        void check_types(AST::Node *lhs, AST::Node *rhs);
        void check_not_null(AST::Node *node);

    public:
        void visit(AST::CodeBlock *block);

        void visit(AST::Identifier *expression);
        void visit(AST::BooleanLiteral *boolean);
        void visit(AST::IntegerLiteral *expression);
        void visit(AST::FloatLiteral *expression);
        void visit(AST::ImaginaryLiteral *imaginary);
        void visit(AST::StringLiteral *expression);
        void visit(AST::SequenceLiteral *sequence);
        void visit(AST::MappingLiteral *mapping);
        void visit(AST::Argument *argument);
        void visit(AST::Call *expression);
        void visit(AST::CCall *ccall);
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
        SymbolTable::Namespace *m_namespace;

    };

};

#endif //JET_TYPING_H
