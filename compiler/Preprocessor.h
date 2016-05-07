//
// Created by Thomas Leese on 25/04/2016.
//

#ifndef JET_GENERICS_H
#define JET_GENERICS_H

#include <map>

#include "ast/visitor.h"

namespace SymbolTable {
    class Namespace;
}

namespace jet {

    namespace ast {
        struct Definition;
        struct Statement;
    }

    namespace preprocessor {

        struct Action {

            enum Kind {
                DropStatement,
                InsertStatement
            };

            Action(Kind kind, ast::Statement *statement = nullptr);

            Kind kind;
            ast::Statement *statement;

        };

        class GenericsPass : public ast::Visitor {

        public:
            GenericsPass(SymbolTable::Namespace *root_namespace);

            void visit(ast::CodeBlock *block);

            void visit(ast::Identifier *identifier);
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
            void visit(ast::Cast *expression);
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
            bool m_collecting;

            std::vector<SymbolTable::Namespace *> m_scope;
            std::vector<Action> m_actions;

            std::map<ast::Definition *, std::vector<std::vector<ast::Identifier *> > > m_generics;
            std::map<SymbolTable::Symbol *, std::string> m_replacements;

            std::vector<ast::Identifier *> m_skip_identifier;

        };

    }

}

#endif //JET_GENERICS_H
