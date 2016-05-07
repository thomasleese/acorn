//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef JET_CODEGENERATOR_H
#define JET_CODEGENERATOR_H

#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "../ast/visitor.h"

namespace jet {

    namespace ast {
        struct Node;
    }

    namespace symboltable {
        class Namespace;
    }

    namespace types {
        class Type;
    }

    namespace codegen {

        class TypeGenerator;
        class TypeInitialiserGenerator;

        class ModuleGenerator : public ast::Visitor {

        public:
            explicit ModuleGenerator(symboltable::Namespace *scope, llvm::DataLayout *data_layout);
            ~ModuleGenerator();

            void debug(std::string line);

            llvm::Module *module() const;

            llvm::Type *generate_type(ast::Node *node, types::Type *type);
            llvm::Type *generate_type(ast::Node *node);

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
            void visit(ast::CCall *expression);
            void visit(ast::Cast *cast);
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
            std::vector<symboltable::Namespace *> m_scope;
            llvm::Module *m_module;
            llvm::IRBuilder<> *m_irBuilder;
            llvm::MDBuilder *m_mdBuilder;
            llvm::DataLayout *m_data_layout;

            std::vector<llvm::Value *> m_llvmValues;

            TypeGenerator *m_type_generator;
        };

    }

}

#endif // JET_CODEGENERATOR_H
