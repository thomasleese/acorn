//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef QUARK_CODEGENERATOR_H
#define QUARK_CODEGENERATOR_H

#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "AbstractSyntaxTree.h"

namespace SymbolTable {
    class Namespace;
}

class CodeGenerator : public AST::Visitor {

public:
    explicit CodeGenerator(SymbolTable::Namespace *rootNamespace);
    ~CodeGenerator();

    void visit(AST::CodeBlock *block);

    void visit(AST::Identifier *expression);
    void visit(AST::IntegerLiteral *expression);
    void visit(AST::StringLiteral *expression);
    void visit(AST::Argument *argument);
    void visit(AST::Call *expression);
    void visit(AST::Assignment *expression);
    void visit(AST::Selector *expression);
    void visit(AST::While *expression);
    void visit(AST::For *expression);
    void visit(AST::If *expression);

    void visit(AST::TypeDeclaration *type);
    void visit(AST::Parameter *parameter);

    void visit(AST::VariableDefinition *definition);
    void visit(AST::FunctionDefinition *definition);
    void visit(AST::TypeDefinition *definition);

    void visit(AST::DefinitionStatement *statement);
    void visit(AST::ExpressionStatement *statement);

    void visit(AST::Module *module);

private:
    SymbolTable::Namespace *m_namespace;
    llvm::Module *m_module;
    llvm::IRBuilder<> *m_irBuilder;
    llvm::MDBuilder *m_mdBuilder;
};


#endif //QUARK_CODEGENERATOR_H
