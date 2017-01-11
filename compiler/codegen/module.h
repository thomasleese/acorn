//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef ACORN_CODEGENERATOR_H
#define ACORN_CODEGENERATOR_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Module.h>

#include "../ast/visitor.h"

namespace acorn {

    namespace ast {
        struct Node;
    }

    namespace diagnostics {
        class Reporter;
    }

    namespace symboltable {
        class Namespace;
    }

    namespace types {
        class Type;
        class Method;
        class ParameterType;
    }

    namespace codegen {

        class BuiltinGenerator;
        class TypeGenerator;

        class Unit;

        std::string mangle_method(std::string name, types::Method *type);

        class ModuleGenerator : public ast::Visitor {

        public:
            ModuleGenerator(diagnostics::Reporter *diagnostics, symboltable::Namespace *scope, llvm::LLVMContext &context, llvm::DataLayout *data_layout);
            ~ModuleGenerator();

            llvm::Module *module() const;

            llvm::Type *generate_type(ast::Node *node, types::Type *type);
            llvm::Type *generate_type(ast::Node *node);

            llvm::Function *generate_function(ast::FunctionDefinition *definition);
            llvm::Function *generate_function(ast::FunctionDefinition *definition, std::map<types::ParameterType *, types::Type *>);

            void push_value(llvm::Value *value);
            llvm::Value *pop_value();

            llvm::BasicBlock *create_basic_block(std::string name) const;

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
            void visit(ast::CCall *expression);
            void visit(ast::Cast *cast);
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
            std::vector<symboltable::Namespace *> m_scope;
            llvm::LLVMContext &m_context;
            llvm::Module *m_module;
            llvm::IRBuilder<> *m_irBuilder;
            llvm::MDBuilder *m_mdBuilder;
            llvm::DataLayout *m_data_layout;

            std::vector<llvm::Value *> m_values;

            BuiltinGenerator *m_builtin_generator;
            TypeGenerator *m_type_generator;

            diagnostics::Reporter *m_diagnostics;
        };

    }

}

#endif // ACORN_CODEGENERATOR_H
