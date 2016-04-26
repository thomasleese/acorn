//
// Created by Thomas Leese on 25/04/2016.
//

#ifndef JET_GENERICS_H
#define JET_GENERICS_H

#include <map>

#include "AbstractSyntaxTree.h"

namespace SymbolTable {
    class Namespace;
}

namespace jet {

namespace preprocessor {

    struct Action {

        enum Kind {
            DropStatement,
            InsertStatement
        };

        Action(Kind kind, AST::Statement *statement = nullptr);

        Kind kind;
        AST::Statement *statement;

    };

    class GenericsPass : public AST::Visitor {

    public:
        GenericsPass(SymbolTable::Namespace *root_namespace);

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

        std::vector<SymbolTable::Namespace *> m_scope;
        std::vector<Action> m_actions;

        std::map<AST::Definition *, std::vector<std::vector<AST::Identifier *> > > m_generics;
        std::map<SymbolTable::Symbol *, std::string> m_replacements;

        AST::Identifier *m_skip_identifier;

    };

}

}

#endif //JET_GENERICS_H
