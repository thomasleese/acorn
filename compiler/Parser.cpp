//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "LexicalAnalysis.h"
#include "Errors.h"
#include "AbstractSyntaxTree.h"
#include "Parser.h"

#include <iostream>

using namespace AST;

Parser::Parser(std::vector<LexicalAnalysis::Token *> tokens) {
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
    Module *module = new Module(m_tokens.front(), name);

    skipToken(LexicalAnalysis::Newline);

    while (!isToken(LexicalAnalysis::EndOfFile)) {
        Statement *statement = readStatement();
        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    //std::cerr << line << std::endl;
}

LexicalAnalysis::Token *Parser::readToken(LexicalAnalysis::Rule rule) {
    LexicalAnalysis::Token *token = m_tokens.front();
    m_tokens.pop_front();

    if (token->rule != rule) {
        throw Errors::SyntaxError(token, rule);
    }

    return token;
}

LexicalAnalysis::Token *Parser::skipToken(LexicalAnalysis::Rule rule) {
    if (isToken(rule)) {
        return readToken(rule);
    } else {
        return 0;
    }
}

bool Parser::isToken(LexicalAnalysis::Rule rule) const {
    if (m_tokens.empty()) {
        return false;
    }

    LexicalAnalysis::Token *token = m_tokens.front();
    return token->rule == rule;
}

CodeBlock *Parser::readCodeBlock() {
    debug("Reading CodeBlock...");

    CodeBlock *code = new CodeBlock(m_tokens.front());

    while (!isToken(LexicalAnalysis::EndKeyword)) {
        Statement *s = readStatement();
        code->statements.push_back(s);
    }

    readToken(LexicalAnalysis::EndKeyword);

    return code;
}

Expression *Parser::readExpression() {
    debug("Reading expression...");

    Expression *expr = readUnaryExpression();
    if (isToken(LexicalAnalysis::Operator) || isToken(LexicalAnalysis::Assignment)) {
        return readBinaryExpression(expr, 0);
    } else {
        return expr;
    }
}

Identifier *Parser::readIdentifier() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::Identifier);
    return new Identifier(token, token->lexeme);
}

Identifier *Parser::readOperator() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::Operator);
    return new Identifier(token, token->lexeme);
}


IntegerLiteral *Parser::readIntegerLiteral() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::IntegerLiteral);

    IntegerLiteral *literal = new IntegerLiteral(token);

    std::stringstream ss;
    ss << token->lexeme;
    ss >> literal->value;

    return literal;
}

FloatLiteral *Parser::readFloatLiteral() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::FloatLiteral);

    FloatLiteral *literal = new FloatLiteral(token);
    literal->value = token->lexeme;

    return literal;
}

StringLiteral *Parser::readStringLiteral() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::StringLiteral);

    StringLiteral *literal = new StringLiteral(token);
    literal->value = token->lexeme;

    return literal;
}

Argument *Parser::readArgument() {
    Argument *argument = new Argument(m_tokens.front());
    argument->name = readIdentifier();
    readToken(LexicalAnalysis::Colon);
    argument->value = readExpression();
    return argument;
}

Call *Parser::readCall(Expression *operand) {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::OpenParenthesis);

    Call *call = new Call(token);
    call->operand = operand;

    while (!isToken(LexicalAnalysis::CloseParenthesis)) {
        call->arguments.push_back(readArgument());

        if (isToken(LexicalAnalysis::Comma)) {
            readToken(LexicalAnalysis::Comma);
        } else {
            break;
        }
    }

    readToken(LexicalAnalysis::CloseParenthesis);

    return call;
}

Selector *Parser::readSelector(AST::Expression *operand) {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::Dot);

    Selector *selector = new Selector(token);
    selector->operand = operand;
    selector->name = readIdentifier();
    return selector;
}

While *Parser::readWhile() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::WhileKeyword);

    While *expression = new While(token);
    expression->condition = readExpression();

    readToken(LexicalAnalysis::Newline);

    expression->code = readCodeBlock();
    return expression;
}

For *Parser::readFor() {
    debug("Reading For...");

    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::ForKeyword);

    For *expression = new For(token);
    expression->name = readIdentifier();

    debug("Name: " + expression->name->name);

    readToken(LexicalAnalysis::InKeyword);
    expression->iterator = readExpression();

    readToken(LexicalAnalysis::Newline);

    expression->code = readCodeBlock();

    debug("Ending For!");

    return expression;
}

If *Parser::readIf() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::IfKeyword);

    If *expression = new If(token);
    expression->condition = readExpression();

    readToken(LexicalAnalysis::Newline);

    expression->trueCode = new CodeBlock(m_tokens.front());
    expression->falseCode = 0;

    while (!isToken(LexicalAnalysis::ElseKeyword) && !isToken(LexicalAnalysis::EndKeyword)) {
        Statement *statement = readStatement();
        expression->trueCode->statements.push_back(statement);
    }

    if (isToken(LexicalAnalysis::ElseKeyword)) {
        readToken(LexicalAnalysis::ElseKeyword);

        if (isToken(LexicalAnalysis::IfKeyword)) {
            readIf();
        } else {
            readToken(LexicalAnalysis::Newline);
            expression->falseCode = readCodeBlock();
        }
    } else {
        readToken(LexicalAnalysis::EndKeyword);
    }

    return expression;
}

Expression *Parser::readUnaryExpression() {
    if (isToken(LexicalAnalysis::Operator)) {
        Call *expr = new Call(m_tokens.front());
        expr->operand = readOperator();

        Argument *arg = new Argument(m_tokens.front());
        arg->name = new Identifier(m_tokens.front(), "self");
        arg->value = readUnaryExpression();
        expr->arguments.push_back(arg);

        return expr;
    } else {
        return readOperandExpression();
    }
}

Expression *Parser::readBinaryExpression(Expression *lhs, int minPrecedence) {
    while ((isToken(LexicalAnalysis::Operator) || isToken(LexicalAnalysis::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] >= minPrecedence) {
        LexicalAnalysis::Token *token = m_tokens.front();

        std::string opName = "=";

        Identifier *op = 0;
        if (isToken(LexicalAnalysis::Operator)) {
            op = readOperator();
            opName = op->name;
        } else {
            readToken(LexicalAnalysis::Assignment);
        }

        Expression *rhs = readOperandExpression();

        while ((isToken(LexicalAnalysis::Operator) || isToken(LexicalAnalysis::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] > m_operatorPrecendence[opName]) {
            rhs = readBinaryExpression(rhs, m_operatorPrecendence[m_tokens.front()->lexeme]);
        }

        if (op) {
            Call *call = new Call(token);
            call->operand = op;

            Argument *lhsArg = new Argument(token, "self");
            lhsArg->value = lhs;
            call->arguments.push_back(lhsArg);

            Argument *rhsArg = new Argument(token, "other");
            rhsArg->value = rhs;
            call->arguments.push_back(rhsArg);

            lhs = call;
        } else {
            Assignment *assignment = new Assignment(token, lhs, rhs);
            lhs = assignment;
        }
    }

    return lhs;
}

Expression *Parser::readOperandExpression() {
    Expression *expr = 0;

    if (isToken(LexicalAnalysis::OpenParenthesis)) {
        readToken(LexicalAnalysis::OpenParenthesis);
        expr = readExpression();
        readToken(LexicalAnalysis::CloseParenthesis);
        return expr;
    } else if (isToken(LexicalAnalysis::IntegerLiteral)) {
        expr = readIntegerLiteral();
    } else if (isToken(LexicalAnalysis::FloatLiteral)) {
        expr = readFloatLiteral();
    } else if (isToken(LexicalAnalysis::StringLiteral)) {
        expr = readStringLiteral();
    } else if (isToken(LexicalAnalysis::WhileKeyword)) {
        expr = readWhile();
    } else if (isToken(LexicalAnalysis::ForKeyword)) {
        expr = readFor();
    } else if (isToken(LexicalAnalysis::IfKeyword)) {
        expr = readIf();
    } else if (isToken(LexicalAnalysis::Identifier)) {
        expr = readIdentifier();
    } else {
        throw Errors::SyntaxError(m_tokens.front(), "operand");
    }

    while (true) {
        if (isToken(LexicalAnalysis::OpenParenthesis)) {
            expr = readCall(expr);
        } else if (isToken(LexicalAnalysis::Dot)) {
            expr = readSelector(expr);
        } else {
            break;
        }
    }

    return expr;
}

TypeDeclaration *Parser::readTypeDeclaration() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::AsKeyword);

    TypeDeclaration *type = new TypeDeclaration(token);
    type->name = readIdentifier();
    return type;
}

Parameter *Parser::readParameter() {
    Parameter *parameter = new Parameter(m_tokens.front());

    parameter->name = readIdentifier();
    parameter->type = readTypeDeclaration();

    if (isToken(LexicalAnalysis::Colon)) {
        readToken(LexicalAnalysis::Colon);
        parameter->defaultExpression = readExpression();
    }

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::LetKeyword);

    VariableDefinition *definition = new VariableDefinition(token);

    definition->name = readIdentifier();
    definition->type = readTypeDeclaration();

    readToken(LexicalAnalysis::Assignment);

    definition->expression = readExpression();

    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (isToken(LexicalAnalysis::Identifier)) {
        definition->name = readIdentifier();
    } else if (isToken(LexicalAnalysis::Operator)) {
        definition->name = readOperator();
    } else {
        throw Errors::SyntaxError(m_tokens.front(), "identifier or operator");
    }

    debug("Name: " + definition->name->name);

    readToken(LexicalAnalysis::OpenParenthesis);

    while (!isToken(LexicalAnalysis::CloseParenthesis)) {
        definition->parameters.push_back(readParameter());

        if (isToken(LexicalAnalysis::Comma)) {
            readToken(LexicalAnalysis::Comma);
        } else {
            break;  // no more parameters, apparently
        }
    }

    readToken(LexicalAnalysis::CloseParenthesis);

    definition->type = readTypeDeclaration();

    readToken(LexicalAnalysis::Newline);

    definition->code = readCodeBlock();

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    LexicalAnalysis::Token *token = readToken(LexicalAnalysis::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition(token);

    definition->name = readIdentifier();

    readToken(LexicalAnalysis::Newline);

    while (!isToken(LexicalAnalysis::EndKeyword)) {
        definition->fields.push_back(readParameter());
        readToken(LexicalAnalysis::Newline);
    }

    readToken(LexicalAnalysis::EndKeyword);

    return definition;
}

Statement *Parser::readStatement() {
    debug("Reading Statement...");

    Statement *statement;

    if (isToken(LexicalAnalysis::LetKeyword)) {
        statement = new DefinitionStatement(readVariableDefinition());
    } else if (isToken(LexicalAnalysis::DefKeyword)) {
        statement = new DefinitionStatement(readFunctionDefinition());
    } else if (isToken(LexicalAnalysis::TypeKeyword)) {
        statement = new DefinitionStatement(readTypeDefinition());
    } else {
        statement = new ExpressionStatement(readExpression());
    }

    readToken(LexicalAnalysis::Newline);

    return statement;
}
