//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <sstream>

#include "Lexer.h"

#include "AbstractSyntaxTree.h"

using namespace AST;

Node::Node(Token *token) {
    this->token = token;
    this->type = nullptr;
}

Node::~Node() {

}

void CodeBlock::accept(Visitor *visitor) {
    visitor->visit(this);
}

Identifier::Identifier(Token *token) : Expression(token) {

}

Identifier::Identifier(Token *token, std::string name) : Expression(token) {
    this->name = name;
}

void Identifier::accept(Visitor *visitor) {
    visitor->visit(this);
}

void BooleanLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void IntegerLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void FloatLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void ImaginaryLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void StringLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void SequenceLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

void MappingLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

Argument::Argument(Token *token) : Node(token) {
    this->name = nullptr;
}

Argument::Argument(Token *token, std::string name) : Node(token) {
    this->name = new Identifier(token, name);
}

void Argument::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call::Call(Token *token) : Expression(token) {

}

Call::Call(std::string name, Token *token) : Expression(token) {
    this->operand = new Identifier(token, name);
}

void Call::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment::Assignment(Token *token, Expression *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

Comma::Comma(Token *token) : Expression(token) {
    this->lhs = nullptr;
    this->rhs = nullptr;
}

Comma::Comma(Expression *lhs, Expression *rhs, Token *token) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Comma::accept(Visitor *visitor) {
    visitor->visit(this);
}

void While::accept(Visitor *visitor) {
    visitor->visit(this);
}

void For::accept(Visitor *visitor) {
    visitor->visit(this);
}

void If::accept(Visitor *visitor) {
    visitor->visit(this);
}

void Return::accept(Visitor *visitor) {
    visitor->visit(this);
}

Type::Type(Token *token) : Expression(token) {

}

Type::Type(std::string name, Token *token) : Expression(token) {
    this->name = new Identifier(token, name);
}

void Type::accept(Visitor *visitor) {
    visitor->visit(this);
}

Cast::Cast(Token *token) : Expression(token) {
    this->typeNode = nullptr;
}

Cast::Cast(Type *typeNode, Token *token) : Expression(token) {
    this->typeNode = typeNode;
}

void Cast::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter::Parameter(Token *token) : Node(token) {
    this->defaultExpression = 0;
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDefinition::VariableDefinition(Token *token) : Definition(token) {
    this->is_mutable = false;
}

VariableDefinition::VariableDefinition(std::string name, Token *token) : Definition(token) {
    this->name = new Identifier(token, name);
    this->is_mutable = false;
}

void VariableDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

void FunctionDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

void TypeDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

DefinitionStatement::DefinitionStatement(Definition *definition) : Statement(definition->token) {
    this->definition = definition;
}

void DefinitionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

ExpressionStatement::ExpressionStatement(Expression *expression) : Statement(expression->token) {
    this->expression = expression;
}

void ExpressionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

Module::Module(Token *token, std::string name) : Node(token) {
    this->name = name;
    this->code = new CodeBlock(token);
}

void Module::accept(Visitor *visitor) {
    visitor->visit(this);
}

Visitor::~Visitor() {

}

void Simplifier::visit(CodeBlock *block) {
    for (long i = 0; i < block->statements.size(); i++) {
        m_insertStatements.clear();
        m_removeStatement = false;

        block->statements[i]->accept(this);

        if (m_removeStatement) {
            block->statements.erase(block->statements.begin() + i);
            i--;
        }

        for (auto statement : m_insertStatements) {
            block->statements.insert(block->statements.begin() + i, statement);
            i++;
        }
    }
}

void Simplifier::visit(Identifier *expression) {

}

void Simplifier::visit(BooleanLiteral *boolean) {

}

void Simplifier::visit(IntegerLiteral *expression) {

}

void Simplifier::visit(FloatLiteral *expression) {

}

void Simplifier::visit(ImaginaryLiteral *expression) {

}

void Simplifier::visit(StringLiteral *expression) {

}

void Simplifier::visit(SequenceLiteral *expression) {

}

void Simplifier::visit(MappingLiteral *expression) {

}

void Simplifier::visit(Argument *expression) {

}

void Simplifier::visit(Call *expression) {

}

void Simplifier::visit(Assignment *expression) {

}

void Simplifier::visit(Selector *expression) {

}

void Simplifier::visit(Comma *expression) {

}

void Simplifier::visit(While *expression) {

}

void Simplifier::visit(For *expression) {
    VariableDefinition *def = new VariableDefinition("state", expression->token);
    def->is_mutable = true;
    def->cast = new Cast(new Type("Integer", expression->token), expression->token);

    Call *call = new Call("start", expression->token);
    call->arguments.push_back(new Argument(expression->token, "self"));
    call->arguments[0]->value = expression->iterator;
    def->expression = call;

    m_insertStatements.push_back(new DefinitionStatement(def));

    While *whileExpr = new While(expression->token);
    call = new Call("not", expression->token);
    call->arguments.push_back(new Argument(expression->token, "self"));
    Call *call2 = new Call("done", expression->token);
    call2->arguments.push_back(new Argument(expression->token, "self"));
    call2->arguments[0]->value = expression->iterator;
    call2->arguments.push_back(new Argument(expression->token, "state"));
    call2->arguments[1]->value = def->name;
    call->arguments[0]->value = call2;
    whileExpr->condition = call;

    whileExpr->code = expression->code;

    m_insertStatements.push_back(new ExpressionStatement(whileExpr));

    Comma *lhs = new Comma(expression->name, def->name, expression->token);
    Call *rhs = new Call("next", expression->token);
    rhs->arguments.push_back(new Argument(expression->token, "self"));
    rhs->arguments[0]->value = expression->iterator;
    rhs->arguments.push_back(new Argument(expression->token, "state"));
    rhs->arguments[1]->value = def->name;
    Assignment *assignment = new Assignment(expression->token, lhs, rhs);
    whileExpr->code->statements.insert(whileExpr->code->statements.begin(), new ExpressionStatement(assignment));

    m_removeStatement = true;
}

void Simplifier::visit(If *expression) {

}

void Simplifier::visit(Return *expression) {

}

void Simplifier::visit(Type *type) {

}

void Simplifier::visit(Cast *type) {

}

void Simplifier::visit(Parameter *parameter) {

}

void Simplifier::visit(VariableDefinition *definition) {

}

void Simplifier::visit(FunctionDefinition *definition) {
    definition->code->accept(this);
}

void Simplifier::visit(TypeDefinition *definition) {

}

void Simplifier::visit(DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Simplifier::visit(ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Simplifier::visit(Module *module) {
    module->code->accept(this);
}
