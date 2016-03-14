//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef QUARK_PRETTYPRINTER_H
#define QUARK_PRETTYPRINTER_H

#include <sstream>
#include <string>

#include "AbstractSyntaxTree.h"

class PrettyPrinter : public AST::Visitor {

public:
    explicit PrettyPrinter();

public:
    void visit(AST::Identifier *expression);
    void visit(AST::IntegerLiteral *expression);
    void visit(AST::StringLiteral *expression);
    void visit(AST::FunctionCall *expression);
    void visit(AST::Selector *expression);

    void visit(AST::TypeDeclaration *type);
    void visit(AST::Parameter *parameter);
    void visit(AST::CodeBlock *block);

    void visit(AST::VariableDefinition *definition);
    void visit(AST::FunctionDefinition *definition);
    void visit(AST::TypeDefinition *definition);

    void visit(AST::DefinitionStatement *statement);
    void visit(AST::ExpressionStatement *statement);

    void visit(AST::Module *module);

private:
    std::string indentation();

public:
    std::string str();
    void print();

private:
    int indent;
    std::stringstream ss;

};

#endif //QUARK_PRETTYPRINTER_H
