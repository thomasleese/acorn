//
// Created by Thomas Leese on 07/05/2016.
//

#ifndef JET_AST_VISITOR_H
#define JET_AST_VISITOR_H

namespace jet {

    namespace ast {

        struct CodeBlock;
        struct Identifier;
        struct BooleanLiteral;
        struct IntegerLiteral;
        struct FloatLiteral;
        struct ImaginaryLiteral;
        struct StringLiteral;
        struct SequenceLiteral;
        struct MappingLiteral;
        struct RecordLiteral;
        struct Call;
        struct CCall;
        struct Cast;
        struct Assignment;
        struct Selector;
        struct Index;
        struct Comma;
        struct While;
        struct For;
        struct If;
        struct Return;
        struct Spawn;
        struct Sizeof;
        struct Strideof;
        struct Parameter;
        struct VariableDefinition;
        struct FunctionDefinition;
        struct TypeDefinition;
        struct DefinitionStatement;
        struct ExpressionStatement;
        struct ImportStatement;
        struct SourceFile;

        class Visitor {
        public:
            virtual ~Visitor();

            // misc
            virtual void visit(CodeBlock *block) = 0;

            // expressions
            virtual void visit(Identifier *identifier) = 0;
            virtual void visit(BooleanLiteral *boolean) = 0;
            virtual void visit(IntegerLiteral *expression) = 0;
            virtual void visit(FloatLiteral *expression) = 0;
            virtual void visit(ImaginaryLiteral *expression) = 0;
            virtual void visit(StringLiteral *expression) = 0;
            virtual void visit(SequenceLiteral *expression) = 0;
            virtual void visit(MappingLiteral *expression) = 0;
            virtual void visit(RecordLiteral *expression) = 0;
            virtual void visit(Call *expression) = 0;
            virtual void visit(CCall *expression) = 0;
            virtual void visit(Cast *expression) = 0;
            virtual void visit(Assignment *expression) = 0;
            virtual void visit(Selector *expression) = 0;
            virtual void visit(Index *expression) = 0;
            virtual void visit(Comma *expression) = 0;
            virtual void visit(While *expression) = 0;
            virtual void visit(For *expression) = 0;
            virtual void visit(If *expression) = 0;
            virtual void visit(Return *expression) = 0;
            virtual void visit(Spawn *expression) = 0;
            virtual void visit(Sizeof *expression) = 0;
            virtual void visit(Strideof *expression) = 0;

            // misc
            virtual void visit(Parameter *parameter) = 0;

            // definitions
            virtual void visit(VariableDefinition *definition) = 0;
            virtual void visit(FunctionDefinition *definition) = 0;
            virtual void visit(TypeDefinition *definition) = 0;

            // statements
            virtual void visit(DefinitionStatement *statement) = 0;
            virtual void visit(ExpressionStatement *statement) = 0;
            virtual void visit(ImportStatement *statement) = 0;

            // module
            virtual void visit(SourceFile *module) = 0;
        };

    }

}

#endif // JET_AST_VISITOR_H
