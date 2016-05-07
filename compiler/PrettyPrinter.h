//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef JET_PRETTYPRINTER_H
#define JET_PRETTYPRINTER_H

#include <sstream>
#include <string>

#include "ast/visitor.h"

class PrettyPrinter : public jet::ast::Visitor {

public:
    explicit PrettyPrinter();

public:
    void visit(jet::ast::CodeBlock *block);

    void visit(jet::ast::Identifier *identifier);
    void visit(jet::ast::BooleanLiteral *boolean);
    void visit(jet::ast::IntegerLiteral *expression);
    void visit(jet::ast::FloatLiteral *expression);
    void visit(jet::ast::ImaginaryLiteral *imaginary);
    void visit(jet::ast::StringLiteral *expression);
    void visit(jet::ast::SequenceLiteral *sequence);
    void visit(jet::ast::MappingLiteral *mapping);
    void visit(jet::ast::RecordLiteral *expression);
    void visit(jet::ast::Argument *argument);
    void visit(jet::ast::Call *expression);
    void visit(jet::ast::CCall *expression);
    void visit(jet::ast::Cast *expression);
    void visit(jet::ast::Assignment *expression);
    void visit(jet::ast::Selector *expression);
    void visit(jet::ast::Index *expression);
    void visit(jet::ast::Comma *expression);
    void visit(jet::ast::While *expression);
    void visit(jet::ast::For *expression);
    void visit(jet::ast::If *expression);
    void visit(jet::ast::Return *expression);
    void visit(jet::ast::Spawn *expression);
    void visit(jet::ast::Sizeof *expression);
    void visit(jet::ast::Strideof *expression);

    void visit(jet::ast::Parameter *parameter);

    void visit(jet::ast::VariableDefinition *definition);
    void visit(jet::ast::FunctionDefinition *definition);
    void visit(jet::ast::TypeDefinition *definition);

    void visit(jet::ast::DefinitionStatement *statement);
    void visit(jet::ast::ExpressionStatement *statement);
    void visit(jet::ast::ImportStatement *statement);

    void visit(jet::ast::SourceFile *module);

private:
    std::string indentation();

public:
    std::string str();
    void print();

private:
    int indent;
    std::stringstream ss;

};

#endif // JET_PRETTYPRINTER_H
