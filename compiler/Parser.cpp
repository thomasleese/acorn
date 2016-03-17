//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "Errors.h"
#include "Lexer.h"
#include "AbstractSyntaxTree.h"
#include "Parser.h"

#include <iostream>

using namespace AST;

Parser::Parser(std::vector<Lexer::Token *> tokens) {
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

    skipToken(Lexer::Newline);

    while (!isToken(Lexer::EndOfFile)) {
        Statement *statement = readStatement();
        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    //std::cerr << line << std::endl;
}

Lexer::Token *Parser::readToken(Lexer::Rule rule) {
    Lexer::Token *token = m_tokens.front();
    m_tokens.pop_front();

    if (token->rule != rule) {
        throw Errors::SyntaxError(token, rule);
    }

    return token;
}

Lexer::Token *Parser::skipToken(Lexer::Rule rule) {
    if (isToken(rule)) {
        return readToken(rule);
    } else {
        return 0;
    }
}

bool Parser::isToken(Lexer::Rule rule) const {
    if (m_tokens.empty()) {
        return false;
    }

    Lexer::Token *token = m_tokens.front();
    return token->rule == rule;
}

CodeBlock *Parser::readCodeBlock() {
    debug("Reading CodeBlock...");

    CodeBlock *code = new CodeBlock(m_tokens.front());

    while (!isToken(Lexer::EndKeyword)) {
        Statement *s = readStatement();
        code->statements.push_back(s);
    }

    readToken(Lexer::EndKeyword);

    return code;
}

Expression *Parser::readExpression() {
    debug("Reading expression...");

    Expression *expr = readUnaryExpression();
    if (isToken(Lexer::Operator) || isToken(Lexer::Assignment)) {
        return readBinaryExpression(expr, 0);
    } else {
        return expr;
    }
}

Identifier *Parser::readIdentifier() {
    Lexer::Token *token = readToken(Lexer::Identifier);
    return new Identifier(token, token->lexeme);
}

Identifier *Parser::readOperator() {
    Lexer::Token *token = readToken(Lexer::Operator);
    return new Identifier(token, token->lexeme);
}


IntegerLiteral *Parser::readIntegerLiteral() {
    Lexer::Token *token = readToken(Lexer::IntegerLiteral);

    IntegerLiteral *literal = new IntegerLiteral(token);

    std::stringstream ss;
    ss << token->lexeme;
    ss >> literal->value;

    return literal;
}

StringLiteral *Parser::readStringLiteral() {
    Lexer::Token *token = readToken(Lexer::StringLiteral);

    StringLiteral *literal = new StringLiteral(token);
    literal->value = token->lexeme;

    return literal;
}

Argument *Parser::readArgument() {
    Argument *argument = new Argument(m_tokens.front());
    argument->name = readIdentifier();
    readToken(Lexer::Colon);
    argument->value = readExpression();
    return argument;
}

Call *Parser::readCall(Expression *operand) {
    Lexer::Token *token = readToken(Lexer::OpenParenthesis);

    Call *call = new Call(token);
    call->operand = operand;

    while (!isToken(Lexer::CloseParenthesis)) {
        call->arguments.push_back(readArgument());

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
    Lexer::Token *token = readToken(Lexer::Dot);

    Selector *selector = new Selector(token);
    selector->operand = operand;
    selector->name = readIdentifier();
    return selector;
}

While *Parser::readWhile() {
    Lexer::Token *token = readToken(Lexer::WhileKeyword);

    While *expression = new While(token);
    expression->condition = readExpression();

    readToken(Lexer::Newline);

    expression->code = readCodeBlock();
    return expression;
}

For *Parser::readFor() {
    debug("Reading For...");

    Lexer::Token *token = readToken(Lexer::ForKeyword);

    For *expression = new For(token);
    expression->name = readIdentifier();

    debug("Name: " + expression->name->name);

    readToken(Lexer::InKeyword);
    expression->iterator = readExpression();

    readToken(Lexer::Newline);

    expression->code = readCodeBlock();

    debug("Ending For!");

    return expression;
}

If *Parser::readIf() {
    Lexer::Token *token = readToken(Lexer::IfKeyword);

    If *expression = new If(token);
    expression->condition = readExpression();

    readToken(Lexer::Newline);

    expression->trueCode = new CodeBlock(m_tokens.front());
    expression->falseCode = 0;

    while (!isToken(Lexer::ElseKeyword) && !isToken(Lexer::EndKeyword)) {
        Statement *statement = readStatement();
        expression->trueCode->statements.push_back(statement);
    }

    if (isToken(Lexer::ElseKeyword)) {
        readToken(Lexer::ElseKeyword);

        if (isToken(Lexer::IfKeyword)) {
            readIf();
        } else {
            readToken(Lexer::Newline);
            expression->falseCode = readCodeBlock();
        }
    } else {
        readToken(Lexer::EndKeyword);
    }

    return expression;
}

Expression *Parser::readUnaryExpression() {
    if (isToken(Lexer::Operator)) {
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
    while ((isToken(Lexer::Operator) || isToken(Lexer::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] >= minPrecedence) {
        Lexer::Token *token = m_tokens.front();

        std::string opName = "=";

        Identifier *op = 0;
        if (isToken(Lexer::Operator)) {
            op = readOperator();
            opName = op->name;
        } else {
            readToken(Lexer::Assignment);
        }

        Expression *rhs = readOperandExpression();

        while ((isToken(Lexer::Operator) || isToken(Lexer::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] > m_operatorPrecendence[opName]) {
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

    if (isToken(Lexer::OpenParenthesis)) {
        readToken(Lexer::OpenParenthesis);
        expr = readExpression();
        readToken(Lexer::CloseParenthesis);
        return expr;
    } else if (isToken(Lexer::IntegerLiteral)) {
        expr = readIntegerLiteral();
    } else if (isToken(Lexer::StringLiteral)) {
        expr = readStringLiteral();
    } else if (isToken(Lexer::WhileKeyword)) {
        expr = readWhile();
    } else if (isToken(Lexer::ForKeyword)) {
        expr = readFor();
    } else if (isToken(Lexer::IfKeyword)) {
        expr = readIf();
    } else if (isToken(Lexer::Identifier)) {
        expr = readIdentifier();
    } else {
        throw Errors::SyntaxError(m_tokens.front(), "operand");
    }

    while (true) {
        if (isToken(Lexer::OpenParenthesis)) {
            expr = readCall(expr);
        } else if (isToken(Lexer::Dot)) {
            expr = readSelector(expr);
        } else {
            break;
        }
    }

    return expr;
}

TypeDeclaration *Parser::readTypeDeclaration() {
    Lexer::Token *token = readToken(Lexer::AsKeyword);

    TypeDeclaration *type = new TypeDeclaration(token);
    type->name = readIdentifier();
    return type;
}

Parameter *Parser::readParameter() {
    Parameter *parameter = new Parameter(m_tokens.front());

    parameter->name = readIdentifier();
    parameter->type = readTypeDeclaration();

    if (isToken(Lexer::Colon)) {
        readToken(Lexer::Colon);
        parameter->defaultExpression = readExpression();
    }

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    Lexer::Token *token = readToken(Lexer::LetKeyword);

    VariableDefinition *definition = new VariableDefinition(token);

    definition->name = readIdentifier();
    definition->type = readTypeDeclaration();

    readToken(Lexer::Assignment);

    definition->expression = readExpression();

    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    Lexer::Token *token = readToken(Lexer::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (isToken(Lexer::Identifier)) {
        definition->name = readIdentifier();
    } else if (isToken(Lexer::Operator)) {
        definition->name = readOperator();
    } else {
        throw Errors::SyntaxError(m_tokens.front(), "identifier or operator");
    }

    debug("Name: " + definition->name->name);

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

    readToken(Lexer::Newline);

    definition->code = readCodeBlock();

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    Lexer::Token *token = readToken(Lexer::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition(token);

    definition->name = readIdentifier();

    readToken(Lexer::Newline);

    while (!isToken(Lexer::EndKeyword)) {
        definition->fields.push_back(readParameter());
        readToken(Lexer::Newline);
    }

    readToken(Lexer::EndKeyword);

    return definition;
}

Statement *Parser::readStatement() {
    debug("Reading Statement...");

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

    readToken(Lexer::Newline);

    return statement;
}
