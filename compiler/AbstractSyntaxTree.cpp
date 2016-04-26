//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"

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

CodeBlock *CodeBlock::clone() const {
    auto new_block = new CodeBlock(this->token);
    new_block->type = this->type;

    for (auto statement : this->statements) {
        new_block->statements.push_back(statement->clone());
    }

    return new_block;
}

Identifier::Identifier(Token *token) : Expression(token) {

}

Identifier::Identifier(Token *token, std::string name) : Expression(token) {
    this->value = name;
}

bool Identifier::has_parameters() const {
    return !parameters.empty();
}

std::string Identifier::collapsed_value() const {
    std::stringstream ss;
    ss << this->value;
    if (has_parameters()) {
        ss << "_";
        for (auto p : this->parameters) {
            ss << p->collapsed_value() << "_";
        }
    }
    return ss.str();
}

void Identifier::collapse_parameters() {
    this->value = collapsed_value();
    this->parameters.clear();
}

void Identifier::accept(Visitor *visitor) {
    visitor->visit(this);
}

Identifier *Identifier::clone() const {
    Identifier *new_identifier = new Identifier(this->token, this->value);
    for (auto parameter : this->parameters) {
        new_identifier->parameters.push_back(parameter->clone());
    }
    new_identifier->type = this->type;
    return new_identifier;
}

void BooleanLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

BooleanLiteral *BooleanLiteral::clone() const {
    auto literal = new BooleanLiteral(this->token);
    literal->type = this->type;
    literal->value = this->value;
    return literal;
}

void IntegerLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

IntegerLiteral *IntegerLiteral::clone() const {
    auto literal = new IntegerLiteral(this->token);
    literal->type = this->type;
    literal->value = this->value;
    return literal;
}

void FloatLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

FloatLiteral *FloatLiteral::clone() const {
    auto literal = new FloatLiteral(this->token);
    literal->type = this->type;
    literal->value = this->value;
    return literal;
}

void ImaginaryLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

ImaginaryLiteral *ImaginaryLiteral::clone() const {
    auto literal = new ImaginaryLiteral(this->token);
    literal->type = this->type;
    literal->value = this->value;
    return literal;
}

void StringLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

StringLiteral *StringLiteral::clone() const {
    auto literal = new StringLiteral(this->token);
    literal->type = this->type;
    literal->value = this->value;
    return literal;
}

void SequenceLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

SequenceLiteral *SequenceLiteral::clone() const {
    auto literal = new SequenceLiteral(this->token);
    literal->type = this->type;

    for (auto element : this->elements) {
        literal->elements.push_back(element->clone());
    }

    return literal;
}

void MappingLiteral::accept(Visitor *visitor) {
    visitor->visit(this);
}

MappingLiteral *MappingLiteral::clone() const {
    auto literal = new MappingLiteral(this->token);
    literal->type = this->type;

    for (auto element : this->keys) {
        literal->keys.push_back(element->clone());
    }

    for (auto element : this->values) {
        literal->values.push_back(element->clone());
    }

    return literal;
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

Argument *Argument::clone() const {
    auto new_argument = new Argument(this->token);
    new_argument->name = this->name->clone();
    new_argument->type = this->type;
    return new_argument;
}

Call::Call(Token *token) : Expression(token) {

}

Call::Call(std::string name, Token *token) : Expression(token) {
    this->operand = new Identifier(token, name);
}

void Call::accept(Visitor *visitor) {
    visitor->visit(this);
}

Call *Call::clone() const {
    auto new_call = new Call(this->token);
    new_call->operand = this->operand->clone();
    new_call->type = this->type;
    return new_call;
}

CCall::CCall(Token *token) : Expression(token) {
    this->name = nullptr;
    this->returnType = nullptr;
}

void CCall::accept(Visitor *visitor) {
    visitor->visit(this);
}

CCall *CCall::clone() const {
    auto new_call = new CCall(this->token);

    new_call->name = this->name->clone();

    for (auto parameter : this->parameters) {
        new_call->parameters.push_back(parameter->clone());
    }

    new_call->returnType = this->returnType->clone();

    for (auto argument : this->arguments) {
        new_call->arguments.push_back(argument->clone());
    }

    new_call->type = this->type;
    return new_call;
}

Assignment::Assignment(Token *token, Identifier *lhs, Expression *rhs) : Expression(token) {
    this->lhs = lhs;
    this->rhs = rhs;
}

void Assignment::accept(Visitor *visitor) {
    visitor->visit(this);
}

Assignment *Assignment::clone() const {
    auto new_assignment = new Assignment(this->token, this->lhs->clone(), this->rhs->clone());
    new_assignment->type = this->type;
    return new_assignment;
}

void Selector::accept(Visitor *visitor) {
    visitor->visit(this);
}

Selector *Selector::clone() const {
    auto new_selector = new Selector(this->token);
    new_selector->operand = this->operand->clone();
    new_selector->name = this->name->clone();
    new_selector->type = this->type;
    return new_selector;
}

void Index::accept(Visitor *visitor) {
    visitor->visit(this);
}

Index *Index::clone() const {
    auto new_index = new Index(this->token);
    new_index->operand = this->operand->clone();
    new_index->index = this->index->clone();
    new_index->type = this->type;
    return new_index;
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

Comma *Comma::clone() const {
    auto new_comma = new Comma(this->lhs->clone(), this->rhs->clone(), this->token);
    new_comma->type = this->type;
    return new_comma;
}

void While::accept(Visitor *visitor) {
    visitor->visit(this);
}

While *While::clone() const {
    While *new_while = new While(this->token);
    new_while->condition = this->condition->clone();
    new_while->code = this->code->clone();
    new_while->type = this->type;
    return new_while;
}

void For::accept(Visitor *visitor) {
    visitor->visit(this);
}

For *For::clone() const {
    auto new_for = new For(this->token);
    new_for->name = this->name->clone();
    new_for->iterator = this->iterator->clone();
    new_for->code = this->code->clone();
    new_for->type = this->type;
    return new_for;
}

void If::accept(Visitor *visitor) {
    visitor->visit(this);
}

If *If::clone() const {
    auto new_if = new If(this->token);
    new_if->condition = this->condition->clone();
    new_if->trueCode = this->trueCode->clone();
    if (this->falseCode) {
        new_if->falseCode = this->falseCode->clone();
    } else {
        new_if->falseCode = nullptr;
    }
    new_if->type = this->type;
    return new_if;
}

void Return::accept(Visitor *visitor) {
    visitor->visit(this);
}

Return *Return::clone() const {
    auto new_return = new Return(this->token);
    new_return->expression = this->expression->clone();
    new_return->type = this->type;
    return new_return;
}

Spawn::Spawn(Token *token, Call *call) : Expression(token) {
    this->call = call;
}

void Spawn::accept(Visitor *visitor) {
    visitor->visit(this);
}

Spawn* Spawn::clone() const {
    auto new_spawn = new Spawn(this->token, this->call->clone());
    new_spawn->type = this->type;
    return new_spawn;
}

Parameter::Parameter(Token *token) : Node(token) {
    this->defaultExpression = nullptr;
}

void Parameter::accept(Visitor *visitor) {
    visitor->visit(this);
}

Parameter* Parameter::clone() const {
    auto new_parameter = new Parameter(this->token);
    new_parameter->name = this->name->clone();
    new_parameter->typeNode = this->typeNode->clone();
    if (this->defaultExpression) {
        new_parameter->defaultExpression = this->defaultExpression->clone();
    }
    new_parameter->type = this->type;
    return new_parameter;
}

VariableDefinition::VariableDefinition(Token *token) : Definition(token) {

}

VariableDefinition::VariableDefinition(std::string name, Token *token) : Definition(token) {
    this->name = new Identifier(token, name);
}

void VariableDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

VariableDefinition* VariableDefinition::clone() const {
    auto new_def = new VariableDefinition(this->token);
    new_def->name = this->name->clone();
    new_def->expression = this->expression->clone();
    if (this->typeNode) {
        new_def->typeNode = this->typeNode->clone();
    }
    return new_def;
}

void FunctionDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

FunctionDefinition* FunctionDefinition::clone() const {
    auto new_def = new FunctionDefinition(this->token);
    new_def->name = this->name->clone();
    new_def->code = this->code->clone();

    for (auto p : this->parameters) {
        new_def->parameters.push_back(p);
    }

    new_def->returnType = this->returnType->clone();
    new_def->type = this->type;
    return new_def;
}

TypeDefinition::TypeDefinition(Token *token) : Definition(token) {
    this->alias = nullptr;
    this->name = nullptr;
}

void TypeDefinition::accept(Visitor *visitor) {
    visitor->visit(this);
}

TypeDefinition* TypeDefinition::clone() const {
    auto new_def = new TypeDefinition(this->token);
    new_def->name = this->name->clone();

    if (this->alias) {
        new_def->alias = this->alias->clone();
    } else {
        for (auto field : this->fields) {
            new_def->fields.push_back(field);
        }
    }

    new_def->type = this->type;
    return new_def;
}

DefinitionStatement::DefinitionStatement(Definition *definition) : Statement(definition->token) {
    this->definition = definition;
}

void DefinitionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

DefinitionStatement* DefinitionStatement::clone() const {
    auto new_statement = new DefinitionStatement(this->definition->clone());
    new_statement->type = this->type;
    return new_statement;
}

ExpressionStatement::ExpressionStatement(Expression *expression) : Statement(expression->token) {
    this->expression = expression;
}

void ExpressionStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

ExpressionStatement* ExpressionStatement::clone() const {
    auto new_statement = new ExpressionStatement(this->expression->clone());
    new_statement->type = this->type;
    return new_statement;
}

ImportStatement::ImportStatement(Token *token, StringLiteral *path) : Statement(token) {
    this->path = path;
}

void ImportStatement::accept(Visitor *visitor) {
    visitor->visit(this);
}

ImportStatement* ImportStatement::clone() const {
    auto new_statement = new ImportStatement(this->token, this->path->clone());
    new_statement->type = this->type;
    return new_statement;
}

SourceFile::SourceFile(Token *token, std::string name) : Node(token) {
    this->name = name;
    this->code = new CodeBlock(token);
}

void SourceFile::accept(Visitor *visitor) {
    visitor->visit(this);
}

SourceFile* SourceFile::clone() const {
    auto new_source_file = new SourceFile(this->token, this->name);
    new_source_file->code = this->code->clone();
    new_source_file->type = this->type;
    return new_source_file;
}

Visitor::~Visitor() {

}
