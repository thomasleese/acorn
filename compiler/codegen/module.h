//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef JET_CODEGENERATOR_H
#define JET_CODEGENERATOR_H

#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "../AbstractSyntaxTree.h"

namespace SymbolTable {
    class Namespace;
}

namespace jet {

    namespace codegen {

        class TypeGenerator;
        class TypeInitialiserGenerator;

        class ModuleGenerator : public AST::Visitor {

        public:
            explicit ModuleGenerator(SymbolTable::Namespace *rootNamespace);
            ~ModuleGenerator();

            void debug(std::string line);

            llvm::Module *module() const;

            llvm::Type *generate_type(AST::Node *node, Types::Type *type);
            llvm::Type *generate_type(AST::Node *node);

            llvm::Constant *generate_initialiser(AST::Node *node, Types::Type *type);
            llvm::Constant *generate_initialiser(AST::Node *node);

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
            void visit(AST::CCall *expression);
            void visit(AST::Cast *cast);
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
            SymbolTable::Namespace *m_scope;
            llvm::Module *m_module;
            llvm::IRBuilder<> *m_irBuilder;
            llvm::MDBuilder *m_mdBuilder;

            std::vector<llvm::Value *> m_llvmValues;

            TypeGenerator *m_type_generator;
            TypeInitialiserGenerator *m_type_initialiser_generator;
        };

    }

}

#endif // JET_CODEGENERATOR_H
