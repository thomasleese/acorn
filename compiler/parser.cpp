//
// Created by Thomas Leese on 13/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "ast/nodes.h"
#include "Errors.h"
#include "Lexer.h"

#include "parser.h"

using namespace jet;
using namespace jet::ast;

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

SourceFile *Parser::parse(std::string name) {
    SourceFile *module = new SourceFile(m_tokens.front(), name);

    skipToken(Token::Newline);

    // read import statements, which must appear at the top of a source file
    while (isToken(Token::ImportKeyword)) {
        module->imports.push_back(readImportStatement());
    }

    // FIXME, implement a proper module system
    for (auto import : module->imports) {
        std::string filename = "stdlib/" + import->path->value + ".jet";

        Lexer lexer;
        std::vector<Token *> tokens = lexer.tokenise(filename);

        if (tokens.size() <= 1) {  // end of file token
            push_error(new errors::FileNotFoundError(import));
            continue;
        }

        Parser parser(tokens);
        SourceFile *module2 = parser.parse(filename);

        for (auto statement : module2->code->statements) {
            module->code->statements.push_back(statement);
        }

        delete module2;
    }

    // read the remaining statements of the file
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
        push_error(new errors::SyntaxError(token, rule));
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

Identifier *Parser::readIdentifier(bool accept_parameters) {
    Token *token = readToken(Token::Identifier);

    auto identifier = new Identifier(token, token->lexeme);

    if (accept_parameters && isToken(Token::OpenBrace)) {
        readToken(Token::OpenBrace);

        while (!isToken(Token::CloseBrace)) {
            identifier->parameters.push_back(readIdentifier(true));

            if (isToken(Token::Comma)) {
                readToken(Token::Comma);
            } else {
                break;
            }
        }

        readToken(Token::CloseBrace);
    }

    return identifier;
}

Identifier *Parser::readOperator(bool accept_parameters) {
    Token *token = readToken(Token::Operator);

    auto identifier = new Identifier(token, token->lexeme);

    if (accept_parameters && isToken(Token::OpenBrace)) {
        readToken(Token::OpenBrace);

        while (!isToken(Token::CloseBrace)) {
            identifier->parameters.push_back(readIdentifier(true));

            if (isToken(Token::Comma)) {
                readToken(Token::Comma);
            } else {
                break;
            }
        }

        readToken(Token::CloseBrace);
    }

    return identifier;
}

BooleanLiteral *Parser::readBooleanLiteral() {
    Token *token = readToken(Token::BooleanLiteral);

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
    literal->value = token->lexeme.substr(1, token->lexeme.length() - 2);

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

RecordLiteral *Parser::readRecordLiteral() {
    Token *token = readToken(Token::NewKeyword);

    auto literal = new RecordLiteral(token);

    literal->name = readIdentifier(true);

    readToken(Token::OpenParenthesis);

    while (!isToken(Token::CloseParenthesis)) {
        literal->field_names.push_back(readIdentifier(false));
        readToken(Token::Colon);
        literal->field_values.push_back(readExpression());

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;
        }
    }

    readToken(Token::CloseParenthesis);

    return literal;
}

Call *Parser::readCall(Expression *operand) {
    Token *token = readToken(Token::OpenParenthesis);

    Call *call = new Call(token);
    call->operand = operand;

    while (!isToken(Token::CloseParenthesis)) {
        call->arguments.push_back(readExpression());

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;
        }
    }

    readToken(Token::CloseParenthesis);

    return call;
}

CCall *Parser::readCCall() {
    Token *token = readToken(Token::CCallKeyword);

    CCall *ccall = new CCall(token);
    ccall->name = readIdentifier(false);

    readToken(Token::OpenParenthesis);

    while (!isToken(Token::CloseParenthesis)) {
        ccall->parameters.push_back(readIdentifier(true));

        if (isToken(Token::Comma)) {
            readToken(Token::Comma);
        } else {
            break;
        }
    }

    readToken(Token::CloseParenthesis);

    readToken(Token::AsKeyword);
    ccall->returnType = readIdentifier(true);

    if (isToken(Token::UsingKeyword)) {
        readToken(Token::UsingKeyword);

        while (true) {
            ccall->arguments.push_back(readExpression());

            if (isToken(Token::Comma)) {
                readToken(Token::Comma);
            } else {
                break;
            }
        }
    }

    return ccall;
}

Cast *Parser::readCast(Expression *operand) {
    Token *token = readToken(Token::AsKeyword);

    Cast *cast = new Cast(token);
    cast->operand = operand;
    cast->new_type = readIdentifier(true);
    return cast;
}

Selector *Parser::readSelector(Expression *operand) {
    Token *token = readToken(Token::Dot);

    Selector *selector = new Selector(token);
    selector->operand = operand;
    selector->name = readIdentifier(true);
    return selector;
}

Call *Parser::readIndex(Expression *operand) {
    Token *token = readToken(Token::OpenBracket);

    auto call = new Call(token);
    call->arguments.push_back(operand);
    call->arguments.push_back(readExpression());

    readToken(Token::CloseBracket);

    if (isToken(Token::Assignment)) {
        readToken(Token::Assignment);
        call->arguments.push_back(readExpression());
        call->operand = new Identifier(token, "setindex!");
    } else {
        call->operand = new Identifier(token, "getindex");
    }

    return call;
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
    expression->name = readIdentifier(false);

    debug("Name: " + expression->name->value);

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
    expression->falseCode = nullptr;

    while (!isToken(Token::ElseKeyword) && !isToken(Token::EndKeyword)) {
        Statement *statement = readStatement();
        expression->trueCode->statements.push_back(statement);
    }

    if (isToken(Token::ElseKeyword)) {
        readToken(Token::ElseKeyword);

        if (isToken(Token::IfKeyword)) {
            expression->falseCode = new CodeBlock(m_tokens.front());
            If *if_expr = readIf();
            expression->falseCode->statements.push_back(new ExpressionStatement(if_expr));
        } else {
            readToken(Token::Newline);
            expression->falseCode = readCodeBlock();
        }
    } else {
        readToken(Token::EndKeyword);
    }

    return expression;
}

Return *Parser::readReturn() {
    Token *token = readToken(Token::ReturnKeyword);

    Return *r = new Return(token);
    r->expression = readExpression();
    return r;
}

Spawn *Parser::readSpawn() {
    Token *token = readToken(Token::SpawnKeyword);

    Expression *expr = readExpression();
    Call *call = dynamic_cast<Call *>(expr);
    if (call) {
        return new Spawn(token, call);
    } else {
        push_error(new errors::SyntaxError(expr->token, "function call"));
        return nullptr;
    }
}

Sizeof *Parser::readSizeof() {
    Token *token = readToken(Token::SizeofKeyword);
    Identifier *identifier = readIdentifier(true);
    return new Sizeof(token, identifier);
}

Strideof *Parser::readStrideof() {
    Token *token = readToken(Token::StrideofKeyword);
    Identifier *identifier = readIdentifier(true);
    return new Strideof(token, identifier);
}

Expression *Parser::readUnaryExpression() {
    if (isToken(Token::Operator)) {
        Call *expr = new Call(m_tokens.front());
        expr->operand = readOperator(true);
        expr->arguments.push_back(readUnaryExpression());

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
            op = readOperator(true);
            opName = op->value;
        } else {
            readToken(Token::Assignment);
        }

        Expression *rhs = readOperandExpression();

        while ((isToken(Token::Operator) || isToken(Token::Assignment)) && m_operatorPrecendence[m_tokens.front()->lexeme] > m_operatorPrecendence[opName]) {
            rhs = readBinaryExpression(rhs, m_operatorPrecendence[m_tokens.front()->lexeme]);
        }

        if (op) {
            auto call = new Call(token);
            call->operand = op;
            call->arguments.push_back(lhs);
            call->arguments.push_back(rhs);

            lhs = call;
        } else {
            lhs = new Assignment(token, lhs, rhs);
        }
    }

    return lhs;
}

Expression *Parser::readPrimaryExpression() {
    if (isToken(Token::OpenParenthesis)) {
        readToken(Token::OpenParenthesis);
        Expression *expr = readExpression();
        readToken(Token::CloseParenthesis);
        return expr;
    } else if (isToken(Token::BooleanLiteral)) {
        return readBooleanLiteral();
    } else if (isToken(Token::IntegerLiteral)) {
        return readIntegerLiteral();
    } else if (isToken(Token::FloatLiteral)) {
        return readFloatLiteral();
    } else if (isToken(Token::ImaginaryLiteral)) {
        return readImaginaryLiteral();
    } else if (isToken(Token::StringLiteral)) {
        return readStringLiteral();
    } else if (isToken(Token::OpenBracket)) {
        return readSequenceLiteral();
    } else if (isToken(Token::OpenBrace)) {
        return readMappingLiteral();
    } else if (isToken(Token::WhileKeyword)) {
        return readWhile();
    } else if (isToken(Token::ForKeyword)) {
        return readFor();
    } else if (isToken(Token::IfKeyword)) {
        return readIf();
    } else if (isToken(Token::ReturnKeyword)) {
        return readReturn();
    } else if (isToken(Token::SpawnKeyword)) {
        return readSpawn();
    } else if (isToken(Token::CCallKeyword)) {
        return readCCall();
    } else if (isToken(Token::SizeofKeyword)) {
        return readSizeof();
    } else if (isToken(Token::StrideofKeyword)) {
        return readStrideof();
    } else if (isToken(Token::NewKeyword)) {
        return readRecordLiteral();
    } else if (isToken(Token::Identifier)) {
        return readIdentifier(true);
    } else if (!m_tokens.empty()) {  // FIXME refactor into function
        push_error(new errors::SyntaxError(m_tokens.front(), "primary expression"));
        return nullptr;
    } else {
        return nullptr;
    }
}

Expression *Parser::readOperandExpression() {
    Expression *left = readPrimaryExpression();

    while (true) {
        if (isToken(Token::OpenParenthesis)) {
            left = readCall(left);
        } else if (isToken(Token::OpenBracket)) {
            left = readIndex(left);
        } else if (isToken(Token::AsKeyword)) {
            left = readCast(left);
        } else if (isToken(Token::Dot)) {
            Selector *selector = readSelector(left);
            if (isToken(Token::OpenParenthesis)) {
                auto call = readCall(selector->name);
                call->arguments.insert(call->arguments.begin(), selector->operand);
                left = call;
            } else {
                left = selector;
            }
        } else {
            break;
        }
    }

    return left;
}

Parameter *Parser::readParameter() {
    Parameter *parameter = new Parameter(m_tokens.front());

    parameter->name = readIdentifier(false);

    readToken(Token::AsKeyword);
    parameter->typeNode = readIdentifier(true);

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    Token *token = readToken(Token::LetKeyword);

    VariableDefinition *definition = new VariableDefinition(token);

    definition->name = readIdentifier(false);

    if (isToken(Token::AsKeyword)) {
        readToken(Token::AsKeyword);
        definition->typeNode = readIdentifier(true);
    } else {
        definition->typeNode = nullptr;
    }

    readToken(Token::Assignment);

    definition->expression = readExpression();

    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    Token *token = readToken(Token::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (isToken(Token::Identifier)) {
        definition->name = readIdentifier(true);
    } else if (isToken(Token::Operator)) {
        definition->name = readOperator(true);
    } else {
        push_error(new errors::SyntaxError(m_tokens.front(), "identifier or operator"));
        return nullptr;
    }

    debug("Name: " + definition->name->value);

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

    readToken(Token::AsKeyword);
    definition->returnType = readIdentifier(true);

    readToken(Token::Newline);

    definition->code = readCodeBlock();

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    Token *token = readToken(Token::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition(token);

    definition->name = readIdentifier(true);

    if (isToken(Token::AsKeyword)) {
        readToken(Token::AsKeyword);
        definition->alias = readIdentifier(true);
    } else {
        readToken(Token::Newline);

        while (!isToken(Token::EndKeyword)) {
            definition->field_names.push_back(readIdentifier(false));
            readToken(Token::AsKeyword);
            definition->field_types.push_back(readIdentifier(true));
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
        auto expression = readExpression();
        if (expression == nullptr) {
            return nullptr;
        }

        statement = new ExpressionStatement(expression);
    }

    readToken(Token::Newline);

    return statement;
}

ImportStatement *Parser::readImportStatement() {
    Token *token = readToken(Token::ImportKeyword);
    StringLiteral *path = readStringLiteral();
    readToken(Token::Newline);

    return new ImportStatement(token, path);
}
