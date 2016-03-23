//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "AbstractSyntaxTree.h"
#include "Errors.h"

#include "Parser.h"

#include <iostream>
#include <cassert>

using namespace AST;

Parser::Parser(std::vector<Token *> tokens) {
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

    skipToken(Token::Newline);

    while (!isToken(Token::EndOfFile)) {
        Statement *statement = readStatement();
        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    //std::cerr << line << std::endl;
}

Token *Parser::readToken(Token::Rule rule) {
    Token *token = m_tokens.front();
    m_tokens.pop_front();

    if (token->rule != rule) {
        throw Errors::SyntaxError(token, rule);
    }

    return token;
}

Token *Parser::skipToken(Token::Rule rule) {
    if (isToken(rule)) {
        return readToken(rule);
    } else {
        return 0;
    }
}

bool Parser::isToken(Token::Rule rule) const {
    if (m_tokens.empty()) {
        return false;
    }

    Token *token = m_tokens.front();
    return token->rule == rule;
}

CodeBlock *Parser::readCodeBlock() {
    debug("Reading CodeBlock...");

    CodeBlock *code = new CodeBlock(m_tokens.front());

    while (!isToken(Token::EndKeyword)) {
        Statement *s = readStatement();
        code->statements.push_back(s);
    }

    readToken(Token::EndKeyword);

    return code;
}

Expression *Parser::readExpression() {
    debug("Reading expression...");

    Expression *expr = readUnaryExpression();
    if (isToken(Token::Operator) || isToken(Token::Assignment)) {
        return readBinaryExpression(expr, 0);
    } else {
        return expr;
    }
}

Identifier *Parser::readIdentifier() {
    Token *token = readToken(Token::Identifier);
    return new Identifier(token, token->lexeme);
}

Identifier *Parser::readOperator() {
    Token *token = readToken(Token::Operator);
    return new Identifier(token, token->lexeme);
}

BooleanLiteral *Parser::readBooleanLiteral() {
    Token *token = readToken(Token::BooleanLiteral);\

    BooleanLiteral *literal = new BooleanLiteral(token);

    if (token->lexeme == "true") {
        literal->value = true;
    } else if (token->lexeme == "false") {
        literal->value = false;
    } else {
        assert(false && "Not here!");
    }

    return literal;
}

IntegerLiteral *Parser::readIntegerLiteral() {
    Token *token = readToken(Token::IntegerLiteral);

    IntegerLiteral *literal = new IntegerLiteral(token);

    std::stringstream ss;
    ss << token->lexeme;
    ss >> literal->value;

    return literal;
}

FloatLiteral *Parser::readFloatLiteral() {
    Token *token = readToken(Token::FloatLiteral);

    FloatLiteral *literal = new FloatLiteral(token);
    literal->value = token->lexeme;

    return literal;
}

ImaginaryLiteral *Parser::readImaginaryLiteral() {
    Token *token = readToken(Token::ImaginaryLiteral);

    ImaginaryLiteral *literal = new ImaginaryLiteral(token);
    literal->value = token->lexeme;

    return literal;
}

StringLiteral *Parser::readStringLiteral() {
    Token *token = readToken(Token::StringLiteral);

    StringLiteral *literal = new StringLiteral(token);
    literal->value = token->lexeme;

    return literal;
}

SequenceLiteral *Parser::readSequenceLiteral() {
    Token *token = readToken(Token::OpenBracket);

    SequenceLiteral *literal = new SequenceLiteral(token);

    while (!isToken(Token::CloseBracket)) {
        literal->elements.push_back(readExpression());

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;
        }
    }

    readToken(Token::CloseBracket);

    return literal;
}

MappingLiteral *Parser::readMappingLiteral() {
    Token *token = readToken(Token::OpenBrace);

    MappingLiteral *literal = new MappingLiteral(token);

    while (!isToken(Token::CloseBrace)) {
        Expression *key = readExpression();
        readToken(Token::Colon);
        Expression *value = readExpression();

        literal->keys.push_back(key);
        literal->values.push_back(value);

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;
        }
    }

    readToken(Token::CloseBrace);

    return literal;
}

Argument *Parser::readArgument() {
    Argument *argument = new Argument(m_tokens.front());
    argument->name = readIdentifier();
    readToken(Token::Colon);
    argument->value = readExpression();
    return argument;
}

Call *Parser::readCall(Expression *operand) {
    Token *token = readToken(Token::OpenParenthesis);

    Call *call = new Call(token);
    call->operand = operand;

    while (!isToken(Token::CloseParenthesis)) {
        call->arguments.push_back(readArgument());

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;
        }
    }

    readToken(Token::CloseParenthesis);

    return call;
}

Selector *Parser::readSelector(AST::Expression *operand) {
    Token *token = readToken(Token::Dot);

    Selector *selector = new Selector(token);
    selector->operand = operand;
    selector->name = readIdentifier();
    return selector;
}

While *Parser::readWhile() {
    Token *token = readToken(Token::WhileKeyword);

    While *expression = new While(token);
    expression->condition = readExpression();

    readToken(Token::Newline);

    expression->code = readCodeBlock();
    return expression;
}

For *Parser::readFor() {
    debug("Reading For...");

    Token *token = readToken(Token::ForKeyword);

    For *expression = new For(token);
    expression->name = readIdentifier();

    debug("Name: " + expression->name->name);

    readToken(Token::InKeyword);
    expression->iterator = readExpression();

    readToken(Token::Newline);

    expression->code = readCodeBlock();

    debug("Ending For!");

    return expression;
}

If *Parser::readIf() {
    Token *token = readToken(Token::IfKeyword);

    If *expression = new If(token);
    expression->condition = readExpression();

    readToken(Token::Newline);

    expression->trueCode = new CodeBlock(m_tokens.front());
    expression->falseCode = 0;

    while (!isToken(Token::ElseKeyword) && !isToken(Token::EndKeyword)) {
        Statement *statement = readStatement();
        expression->trueCode->statements.push_back(statement);
    }

    if (isToken(Token::ElseKeyword)) {
        readToken(Token::ElseKeyword);

        if (isToken(Token::IfKeyword)) {
            readIf();
        } else {
            readToken(Token::Newline);
            expression->falseCode = readCodeBlock();
        }
    } else {
        readToken(Token::EndKeyword);
    }

    return expression;
}

Expression *Parser::readUnaryExpression() {
    if (isToken(Token::Operator)) {
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
    while ((isToken(Token::Operator) || isToken(Token::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] >= minPrecedence) {
        Token *token = m_tokens.front();

        std::string opName = "=";

        Identifier *op = 0;
        if (isToken(Token::Operator)) {
            op = readOperator();
            opName = op->name;
        } else {
            readToken(Token::Assignment);
        }

        Expression *rhs = readOperandExpression();

        while ((isToken(Token::Operator) || isToken(Token::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] > m_operatorPrecendence[opName]) {
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

    if (isToken(Token::OpenParenthesis)) {
        readToken(Token::OpenParenthesis);
        expr = readExpression();
        readToken(Token::CloseParenthesis);
        return expr;
    } else if (isToken(Token::BooleanLiteral)) {
        expr = readBooleanLiteral();
    } else if (isToken(Token::IntegerLiteral)) {
        expr = readIntegerLiteral();
    } else if (isToken(Token::FloatLiteral)) {
        expr = readFloatLiteral();
    } else if (isToken(Token::ImaginaryLiteral)) {
        expr = readImaginaryLiteral();
    } else if (isToken(Token::StringLiteral)) {
        expr = readStringLiteral();
    } else if (isToken(Token::OpenBracket)) {
        expr = readSequenceLiteral();
    } else if (isToken(Token::OpenBrace)) {
        expr = readMappingLiteral();
    } else if (isToken(Token::WhileKeyword)) {
        expr = readWhile();
    } else if (isToken(Token::ForKeyword)) {
        expr = readFor();
    } else if (isToken(Token::IfKeyword)) {
        expr = readIf();
    } else if (isToken(Token::Identifier)) {
        expr = readIdentifier();
    } else {
        throw Errors::SyntaxError(m_tokens.front(), "operand");
    }

    while (true) {
        if (isToken(Token::OpenParenthesis)) {
            expr = readCall(expr);
        } else if (isToken(Token::Dot)) {
            expr = readSelector(expr);
        } else {
            break;
        }
    }

    return expr;
}

Type *Parser::readType() {
    Type *type = new Type(m_tokens.front());

    type->name = readIdentifier();

    if (isToken(Token::OpenBrace)) {
        readToken(Token::OpenBrace);

        while (!isToken(Token::CloseBrace)) {
            type->parameters.push_back(readType());

            if (isToken(Token::Comma)) {
                readToken(Token::Comma);
            } else {
                break;
            }
        }

        readToken(Token::CloseBrace);
    }

    return type;
}

Cast *Parser::readCast() {
    Token *token = readToken(Token::AsKeyword);

    Cast *cast = new Cast(token);
    cast->typeNode = readType();
    return cast;
}

Parameter *Parser::readParameter() {
    Parameter *parameter = new Parameter(m_tokens.front());

    parameter->name = readIdentifier();
    parameter->cast = readCast();

    if (isToken(Token::Colon)) {
        readToken(Token::Colon);
        parameter->defaultExpression = readExpression();
    }

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    Token *token = readToken(Token::LetKeyword);

    VariableDefinition *definition = new VariableDefinition(token);

    if (isToken(Token::MutableKeyword)) {
        readToken(Token::MutableKeyword);
        definition->is_mutable = true;
    } else {
        definition->is_mutable = false;
    }

    definition->name = readIdentifier();

    definition->cast = readCast();

    readToken(Token::Assignment);

    definition->expression = readExpression();

    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    Token *token = readToken(Token::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (isToken(Token::Identifier)) {
        definition->name = readIdentifier();
    } else if (isToken(Token::Operator)) {
        definition->name = readOperator();
    } else {
        throw Errors::SyntaxError(m_tokens.front(), "identifier or operator");
    }

    debug("Name: " + definition->name->name);

    readToken(Token::OpenParenthesis);

    while (!isToken(Token::CloseParenthesis)) {
        definition->parameters.push_back(readParameter());

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;  // no more parameters, apparently
        }
    }

    readToken(Token::CloseParenthesis);

    definition->returnCast = readCast();

    readToken(Token::Newline);

    definition->code = readCodeBlock();

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    Token *token = readToken(Token::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition(token);

    definition->name = readType();

    if (isToken(Token::AsKeyword)) {
        readToken(Token::AsKeyword);
        definition->alias = readType();
    } else {
        readToken(Token::Newline);

        while (!isToken(Token::EndKeyword)) {
            definition->fields.push_back(readParameter());
            readToken(Token::Newline);
        }

        readToken(Token::EndKeyword);
    }

    return definition;
}

Statement *Parser::readStatement() {
    debug("Reading Statement...");

    Statement *statement;

    if (isToken(Token::LetKeyword)) {
        statement = new DefinitionStatement(readVariableDefinition());
    } else if (isToken(Token::DefKeyword)) {
        statement = new DefinitionStatement(readFunctionDefinition());
    } else if (isToken(Token::TypeKeyword)) {
        statement = new DefinitionStatement(readTypeDefinition());
    } else {
        statement = new ExpressionStatement(readExpression());
    }

    readToken(Token::Newline);

    return statement;
}
