//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "Lexer.h"
#include "AbstractSyntaxTree.h"
#include "Parser.h"

#include <iostream>

using namespace AST;

Parser::Parser(std::vector<Lexer::Token> tokens) {
    for (auto token : tokens) {
        m_tokens.push_back(token);
    }

    m_operatorPrecendence["="] = 0;

    m_operatorPrecendence["+"] = 1;
    m_operatorPrecendence["-"] = 1;
}

Parser::~Parser() {

}

Module *Parser::parse(std::string name) {
    Module *module = new Module(name);

    while (!m_tokens.empty()) {
        Statement *statement = readStatement();
        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::skipNewlines() {
    while (isToken(Lexer::Newline)) {
        readToken(Lexer::Newline);
    }
}

void Parser::readNewlines() {
    readToken(Lexer::Newline);
    skipNewlines();
}

Lexer::Token Parser::readToken(Lexer::Rule rule) {
    Lexer::Token token = m_tokens.front();
    m_tokens.pop_front();

    if (token.rule != rule) {
        throw std::logic_error(std::string("Unexpected token: ") + token.lexeme);
    }

    return token;
}

bool Parser::isToken(Lexer::Rule rule) const {
    Lexer::Token token = m_tokens.front();
    return token.rule == rule;
}

Expression *Parser::readExpression() {
    Expression *expr = readUnaryExpression();
    if (isToken(Lexer::Operator)) {
        return readBinaryExpression(expr, 0);
    } else {
        return expr;
    }
}

Identifier *Parser::readIdentifier() {
    Identifier *identifier = new Identifier();
    identifier->name = readToken(Lexer::Identifier).lexeme;
    return identifier;
}

Identifier *Parser::readOperator() {
    Identifier *identifier = new Identifier();
    identifier->name = readToken(Lexer::Operator).lexeme;
    return identifier;
}


IntegerLiteral *Parser::readIntegerLiteral() {
    Lexer::Token token = readToken(Lexer::IntegerLiteral);

    IntegerLiteral *literal = new IntegerLiteral();

    std::stringstream ss;
    ss << token.lexeme;
    ss >> literal->value;

    return literal;
}

StringLiteral *Parser::readStringLiteral() {
    Lexer::Token token = readToken(Lexer::StringLiteral);

    StringLiteral *literal = new StringLiteral();
    literal->value = token.lexeme;

    return literal;
}

FunctionCall *Parser::readFunctionCall(Expression *operand) {
    readToken(Lexer::OpenParenthesis);

    FunctionCall *call = new FunctionCall();
    call->operand = operand;

    while (!isToken(Lexer::CloseParenthesis)) {
        Identifier *name = readIdentifier();
        readToken(Lexer::Colon);
        Expression *value = readExpression();

        call->arguments[name] = value;

        if (isToken(Lexer::Comma)) {
            readToken(Lexer::Comma);
        } else {
            break;
        }
    }

    readToken(Lexer::CloseParenthesis);

    return call;
}

Selector *Parser::readSelector(AST::Expression *operand) {
    readToken(Lexer::Dot);

    Selector *selector = new Selector();
    selector->operand = operand;
    selector->name = readIdentifier();
    return selector;
}

Expression *Parser::readUnaryExpression() {
    if (isToken(Lexer::Operator)) {
        FunctionCall *expr = new FunctionCall();
        expr->operand = readOperator();

        Identifier *arg = new Identifier();
        arg->name = "a";
        expr->arguments[arg] = readUnaryExpression();
        return expr;
    } else {
        return readOperandExpression();
    }
}

Expression *Parser::readBinaryExpression(Expression *lhs, int minPrecedence) {
    while (isToken(Lexer::Operator) && m_operatorPrecendence[m_tokens.front().lexeme] >= minPrecedence) {
        Identifier *op = readOperator();
        Expression *rhs = readOperandExpression();

        while (isToken(Lexer::Operator) && m_operatorPrecendence[m_tokens.front().lexeme] > m_operatorPrecendence[op->name]) {
            rhs = readBinaryExpression(rhs, m_operatorPrecendence[m_tokens.front().lexeme]);
        }

        FunctionCall *expr = new FunctionCall();
        expr->operand = op;

        Identifier *lhsArg = new Identifier();
        lhsArg->name = "a";

        Identifier *rhsArg = new Identifier();
        rhsArg->name = "b";

        expr->arguments[lhsArg] = lhs;
        expr->arguments[rhsArg] = rhs;
        lhs = expr;
    }

    return lhs;
}

Expression *Parser::readOperandExpression() {
    Expression *expr = 0;

    if (isToken(Lexer::OpenParenthesis)) {
        readToken(Lexer::OpenParenthesis);
        expr = readExpression();
        readToken(Lexer::CloseParenthesis);
        return expr;
    } else if (isToken(Lexer::IntegerLiteral)) {
        expr = readIntegerLiteral();
    } else if (isToken(Lexer::StringLiteral)) {
        expr = readStringLiteral();
    } else if (isToken(Lexer::Identifier)) {
        expr = readIdentifier();
    } else {
        throw std::logic_error("Expected operand.");
    }

    while (true) {
        if (isToken(Lexer::OpenParenthesis)) {
            expr = readFunctionCall(expr);
        } else if (isToken(Lexer::Dot)) {
            expr = readSelector(expr);
        } else {
            break;
        }
    }

    return expr;
}

TypeDeclaration *Parser::readTypeDeclaration() {
    readToken(Lexer::AsKeyword);

    TypeDeclaration *type = new TypeDeclaration();
    type->name = readIdentifier();
    return type;
}

Parameter Parser::readParameter() {
    Parameter parameter;

    parameter.name = readIdentifier();
    parameter.type = readTypeDeclaration();

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    readToken(Lexer::LetKeyword);

    VariableDefinition *definition = new VariableDefinition();

    definition->name = readIdentifier();
    definition->type = readTypeDeclaration();

    Identifier *op = readOperator();
    if (op->name != "=") {
        throw std::logic_error("Expected assignment operator.");
    }
    delete op;

    definition->expression = readExpression();

    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    readToken(Lexer::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition();

    if (isToken(Lexer::Identifier)) {
        definition->name = readIdentifier();
    } else if (isToken(Lexer::Operator)) {
        definition->name = readOperator();
    } else {
        throw std::logic_error("Expected an identifier or operator.");
    }

    readToken(Lexer::OpenParenthesis);

    while (!isToken(Lexer::CloseParenthesis)) {
        definition->parameters.push_back(readParameter());

        if (isToken(Lexer::Comma)) {
            readToken(Lexer::Comma);
        } else {
            break;  // no more parameters, apparently
        }
    }

    readToken(Lexer::CloseParenthesis);

    definition->type = readTypeDeclaration();

    readNewlines();

    while (!isToken(Lexer::EndKeyword)) {
        Statement *s = readStatement();
        definition->code.statements.push_back(s);
    }

    readToken(Lexer::EndKeyword);

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    readToken(Lexer::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition();

    definition->name = readIdentifier();

    readNewlines();

    while (!isToken(Lexer::EndKeyword)) {
        Identifier *fieldName = readIdentifier();
        TypeDeclaration *fieldType = readTypeDeclaration();
        readNewlines();

        definition->fields[fieldName] = fieldType;
    }

    readToken(Lexer::EndKeyword);

    return definition;
}

Statement *Parser::readStatement() {
    Statement *statement;

    if (isToken(Lexer::LetKeyword)) {
        statement = new DefinitionStatement(readVariableDefinition());
    } else if (isToken(Lexer::DefKeyword)) {
        statement = new DefinitionStatement(readFunctionDefinition());
    } else if (isToken(Lexer::TypeKeyword)) {
        statement = new DefinitionStatement(readTypeDefinition());
    } else {
        statement = new ExpressionStatement(readExpression());
    }

    readNewlines();

    return statement;
}
