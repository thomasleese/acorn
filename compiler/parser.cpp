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

using namespace acorn;
using namespace acorn::ast;

#define return_if_null(thing) if (thing == nullptr) return nullptr;

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
        std::string filename = "stdlib/" + import->path->value + ".acorn";

        Lexer lexer;
        std::vector<Token *> tokens = lexer.tokenise(filename);

        if (tokens.size() <= 1) {  // end of file token
            push_error(new errors::FileNotFoundError(import));
            continue;
        }

        Parser parser(tokens);
        auto module2 = parser.parse(filename);

        if (parser.has_errors()) {
            while (parser.has_errors()) {
                push_error(parser.next_error());
            }
        } else {
            for (auto statement : module2->code->statements) {
                module->code->statements.push_back(statement);
            }
        }

        delete module2;
    }

    // read the remaining statements of the file
    while (!isToken(Token::EndOfFile)) {
        auto statement = readStatement();
        if (statement == nullptr) break;

        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    //std::cerr << line << std::endl;
}

Token *Parser::readToken(Token::Rule rule) {
    auto token = m_tokens.front();
    m_tokens.pop_front();

    if (token->rule != rule) {
        push_error(new errors::SyntaxError(token, rule));
        return nullptr;
    }

    return token;
}

Token *Parser::skipToken(Token::Rule rule) {
    if (isToken(rule)) {
        return readToken(rule);
    } else {
        return nullptr;
    }
}

bool Parser::isToken(Token::Rule rule) const {
    if (m_tokens.empty()) {
        return false;
    }

    auto token = m_tokens.front();
    return token->rule == rule;
}

CodeBlock *Parser::readCodeBlock() {
    auto code = new CodeBlock(m_tokens.front());

    while (!isToken(Token::EndKeyword)) {
        auto statement = readStatement();
        if (statement == nullptr) {
            break;
        }

        code->statements.push_back(statement);
    }

    readToken(Token::EndKeyword);

    return code;
}

Expression *Parser::readExpression(bool parse_comma) {
    debug("Reading expression...");

    auto expr = readUnaryExpression(parse_comma);
    if (expr == nullptr) {
        return nullptr;
    }

    if (isToken(Token::Operator) || isToken(Token::Assignment)) {
        return readBinaryExpression(expr, 0);
    } else {
        return expr;
    }
}

Identifier *Parser::readIdentifier(bool accept_parameters) {
    auto token = readToken(Token::Identifier);
    return_if_null(token);

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

ast::VariableDeclaration *Parser::readVariableDeclaration() {
    auto token = readToken(Token::LetKeyword);
    return_if_null(token);

    auto name = readIdentifier(false);
    return_if_null(name);

    ast::Identifier *type = nullptr;
    if (isToken(Token::AsKeyword)) {
        readToken(Token::AsKeyword);

        type = readIdentifier(true);
        return_if_null(type);
    }

    return new VariableDeclaration(token, name, type);
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
        literal->elements.push_back(readExpression(false));

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
        Expression *key = readExpression(false);
        readToken(Token::Colon);
        Expression *value = readExpression(false);

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
        literal->field_values.push_back(readExpression(false));

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
        call->arguments.push_back(readExpression(false));

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
            ccall->arguments.push_back(readExpression(false));

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
    return_if_null(token);

    Identifier *name = nullptr;

    if (isToken(Token::IntegerLiteral)) {
        auto il = readIntegerLiteral();
        name = new Identifier(il->token, il->value);
        delete il;
    } else {
        name = readIdentifier(true);
    }

    return_if_null(name);

    return new Selector(token, operand, name);
}

Call *Parser::readIndex(Expression *operand) {
    Token *token = readToken(Token::OpenBracket);

    auto call = new Call(token);
    call->arguments.push_back(operand);
    call->arguments.push_back(readExpression(true));

    readToken(Token::CloseBracket);

    if (isToken(Token::Assignment)) {
        readToken(Token::Assignment);
        call->arguments.push_back(readExpression(true));
        call->operand = new Identifier(token, "setindex");
    } else {
        call->operand = new Identifier(token, "getindex");
    }

    return call;
}

While *Parser::readWhile() {
    auto token = readToken(Token::WhileKeyword);
    return_if_null(token);

    auto condition = readExpression(true);
    return_if_null(condition);

    return_if_null(readToken(Token::DoKeyword));
    return_if_null(readToken(Token::Newline));

    auto code = readCodeBlock();
    return_if_null(code);

    return new While(token, condition, code);
}

CodeBlock *Parser::readFor() {
    Token *token = readToken(Token::ForKeyword);
    return_if_null(token);

    auto variable = readIdentifier(false);
    return_if_null(variable);

    return_if_null(readToken(Token::InKeyword));

    auto iterator = readExpression(true);
    return_if_null(iterator);

    return_if_null(readToken(Token::DoKeyword));
    return_if_null(readToken(Token::Newline));

    auto loop_code = readCodeBlock();
    return_if_null(loop_code);

    std::string for_id;
    std::stringstream ss;
    ss << token;
    ss >> for_id;

    std::string state_variable_name = "state_" + for_id;
    std::string next_state_variable_name = "next_state_" + for_id;

    auto code_block = new CodeBlock(token);

    auto state_variable = new VariableDefinition(token, state_variable_name, new Call(token, "start", iterator));
    code_block->statements.push_back(new DefinitionStatement(state_variable));

    auto condition = new Call(token, "not", new Call(token, "done", iterator, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name()));
    auto while_code = new While(token, condition, loop_code);

    auto next_state_variable = new VariableDefinition(token, next_state_variable_name, new Call(token, "next", iterator, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name()));
    loop_code->statements.insert(loop_code->statements.begin(), new DefinitionStatement(next_state_variable));
    loop_code->statements.insert(loop_code->statements.begin() + 1, new ExpressionStatement(new Assignment(token, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name(), new Selector(token, static_cast<ast::VariableDeclaration *>(next_state_variable->assignment->lhs)->name(), "1"))));
    loop_code->statements.insert(loop_code->statements.begin() + 1, new ExpressionStatement(new Assignment(token, variable, new Selector(token, static_cast<ast::VariableDeclaration *>(next_state_variable->assignment->lhs)->name(), "0"))));

    code_block->statements.push_back(new ExpressionStatement(while_code));

    return code_block;
}

If *Parser::readIf() {
    auto token = readToken(Token::IfKeyword);

    If *expression = new If(token);

    if (isToken(Token::LetKeyword)) {
        auto lhs = readVariableDeclaration();
        return_if_null(lhs);

        auto assignment_token = readToken(Token::Assignment);
        return_if_null(assignment_token);

        auto rhs = readExpression(true);
        return_if_null(rhs);

        expression->condition = new Assignment(assignment_token, lhs, rhs);
    } else {
        expression->condition = readExpression(true);
        return_if_null(expression->condition);
    }

    readToken(Token::ThenKeyword);

    return_if_null(readToken(Token::Newline));

    expression->trueCode = new CodeBlock(m_tokens.front());
    expression->falseCode = nullptr;

    while (!isToken(Token::ElseKeyword) && !isToken(Token::EndKeyword)) {
        auto statement = readStatement();
        if (statement == nullptr) {
            break;
        }

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
    r->expression = readExpression(true);
    return r;
}

Spawn *Parser::readSpawn() {
    Token *token = readToken(Token::SpawnKeyword);

    Expression *expr = readExpression(true);
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

Expression *Parser::readUnaryExpression(bool parse_comma) {
    if (isToken(Token::Operator)) {
        Call *expr = new Call(m_tokens.front());
        expr->operand = readOperator(true);
        expr->arguments.push_back(readUnaryExpression(false));

        return expr;
    } else {
        return readOperandExpression(parse_comma);
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

        Expression *rhs = readOperandExpression(true);

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
        Expression *expr = readExpression(true);
        readToken(Token::CloseParenthesis);
        return expr;
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

Expression *Parser::readOperandExpression(bool parse_comma) {
    auto left = readPrimaryExpression();
    if (left == nullptr) {
        return nullptr;
    }

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
        } else if (parse_comma && isToken(Token::Comma)) {
            readToken(Token::Comma);

            auto rhs = readExpression(true);
            std::vector<Expression *> elements;

            elements.push_back(left);

            if (auto tuple = dynamic_cast<TupleLiteral *>(rhs)) {
                for (auto e : tuple->elements()) {
                    elements.push_back(e);
                }
            } else {
                elements.push_back(rhs);
            }

            left = new TupleLiteral(left->token, elements);
        } else {
            break;
        }
    }

    return left;
}

Parameter *Parser::readParameter() {
    auto parameter = new Parameter(m_tokens.front());

    if (isToken(Token::InoutKeyword)) {
        readToken(Token::InoutKeyword);
        parameter->inout = true;
    } else {
        parameter->inout = false;
    }

    parameter->name = readIdentifier(false);

    readToken(Token::AsKeyword);
    parameter->typeNode = readIdentifier(true);

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    auto lhs = readVariableDeclaration();
    return_if_null(lhs);

    auto token = readToken(Token::Assignment);
    return_if_null(token);

    auto rhs = readExpression(true);
    return_if_null(rhs);

    auto definition = new VariableDefinition(lhs->token);
    definition->name = lhs->name();
    definition->assignment = new Assignment(token, lhs, rhs);
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
        auto definition = readVariableDefinition();
        return_if_null(definition);
        statement = new DefinitionStatement(definition);
    } else if (isToken(Token::DefKeyword)) {
        statement = new DefinitionStatement(readFunctionDefinition());
    } else if (isToken(Token::TypeKeyword)) {
        statement = new DefinitionStatement(readTypeDefinition());
    } else {
        auto expression = readExpression(true);
        return_if_null(expression);

        statement = new ExpressionStatement(expression);
    }

    return_if_null(readToken(Token::Newline));

    return statement;
}

ImportStatement *Parser::readImportStatement() {
    Token *token = readToken(Token::ImportKeyword);
    StringLiteral *path = readStringLiteral();
    readToken(Token::Newline);

    return new ImportStatement(token, path);
}
