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
    Module *module = new Module(name);

    skipNewlines();

    while (!isToken(Lexer::EndOfFile)) {
        Statement *statement = readStatement();
        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    //std::cerr << line << std::endl;
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
    Lexer::Token *token = m_tokens.front();
    m_tokens.pop_front();

    if (token->rule != rule) {
        throw Errors::SyntaxError(token, rule);
    }

    return *token;
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

    CodeBlock *code = new CodeBlock();

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

Argument *Parser::readArgument() {
    Argument *argument = new Argument();
    argument->name = readIdentifier();
    readToken(Lexer::Colon);
    argument->value = readExpression();
    return argument;
}

Call *Parser::readCall(Expression *operand) {
    readToken(Lexer::OpenParenthesis);

    Call *call = new Call();
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
    readToken(Lexer::Dot);

    Selector *selector = new Selector();
    selector->operand = operand;
    selector->name = readIdentifier();
    return selector;
}

While *Parser::readWhile() {
    readToken(Lexer::WhileKeyword);

    While *expression = new While();
    expression->condition = readExpression();

    readNewlines();
    expression->code = readCodeBlock();
    return expression;
}

For *Parser::readFor() {
    debug("Reading For...");

    readToken(Lexer::ForKeyword);

    For *expression = new For();
    expression->name = readIdentifier();

    debug("Name: " + expression->name->name);

    readToken(Lexer::InKeyword);
    expression->iterator = readExpression();

    readNewlines();

    expression->code = readCodeBlock();

    debug("Ending For!");

    return expression;
}

If *Parser::readIf() {
    readToken(Lexer::IfKeyword);

    If *expression = new If();
    expression->condition = readExpression();
    readNewlines();

    expression->trueCode = new CodeBlock();
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
            readNewlines();
            expression->falseCode = readCodeBlock();
        }
    } else {
        readToken(Lexer::EndKeyword);
    }

    return expression;
}

Expression *Parser::readUnaryExpression() {
    if (isToken(Lexer::Operator)) {
        Call *expr = new Call();
        expr->operand = readOperator();

        Argument *arg = new Argument();
        arg->name = new Identifier("self");
        arg->value = readUnaryExpression();
        expr->arguments.push_back(arg);

        return expr;
    } else {
        return readOperandExpression();
    }
}

Expression *Parser::readBinaryExpression(Expression *lhs, int minPrecedence) {
    while (isToken(Lexer::Operator) && m_operatorPrecendence[m_tokens.front()->lexeme] >= minPrecedence) {
        Identifier *op = readOperator();
        Expression *rhs = readOperandExpression();

        while (isToken(Lexer::Operator) && m_operatorPrecendence[m_tokens.front()->lexeme] > m_operatorPrecendence[op->name]) {
            rhs = readBinaryExpression(rhs, m_operatorPrecendence[m_tokens.front()->lexeme]);
        }

        Call *expr = new Call();
        expr->operand = op;

        Argument *lhsArg = new Argument("self");
        lhsArg->value = lhs;
        expr->arguments.push_back(lhsArg);

        Argument *rhsArg = new Argument("other");
        rhsArg->value = rhs;
        expr->arguments.push_back(rhsArg);

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
    readToken(Lexer::AsKeyword);

    TypeDeclaration *type = new TypeDeclaration();
    type->name = readIdentifier();
    return type;
}

Parameter *Parser::readParameter() {
    Parameter *parameter = new Parameter();

    parameter->name = readIdentifier();
    parameter->type = readTypeDeclaration();

    if (isToken(Lexer::Colon)) {
        readToken(Lexer::Colon);
        parameter->defaultExpression = readExpression();
    }

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    readToken(Lexer::LetKeyword);

    VariableDefinition *definition = new VariableDefinition();

    definition->name = readIdentifier();
    definition->type = readTypeDeclaration();

    Lexer::Token *token = m_tokens.front();
    Identifier *op = readOperator();
    if (op->name != "=") {
        throw Errors::SyntaxError(token, "=");
    }
    delete op;

    definition->expression = readExpression();

    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    readToken(Lexer::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition();

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

    readNewlines();

    while (!isToken(Lexer::EndKeyword)) {
        Statement *s = readStatement();
        definition->code.statements.push_back(s);
    }

    readToken(Lexer::EndKeyword);

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    readToken(Lexer::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition();

    definition->name = readIdentifier();

    readNewlines();

    while (!isToken(Lexer::EndKeyword)) {
        definition->fields.push_back(readParameter());
        readNewlines();
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

    readNewlines();

    return statement;
}
