//
// Created by Thomas Leese on 13/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "ast.h"
#include "diagnostics.h"
#include "lexer.h"

#include "parser.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::ast;

#define return_if_null(thing) if (thing == nullptr) return nullptr;
#define return_if_false(thing) if (thing == false) return nullptr;

// useful variable for storing the current token
static Token token;

Parser::Parser(Lexer &lexer)
        : m_lexer(lexer)
{
    m_operatorPrecendence["="] = 0;

    m_operatorPrecendence["+"] = 1;
    m_operatorPrecendence["-"] = 1;
}

Parser::~Parser() {

}

SourceFile *Parser::parse(std::string name) {
    return_if_false(fill_token())

    auto module = new SourceFile(m_tokens.front(), name);

    // read import statements, which must appear at the top of a source file
    while (is_token(Token::ImportKeyword)) {
        module->imports.push_back(readImportStatement());
    }

    // FIXME, implement a proper module system
    for (auto import : module->imports) {
        std::string filename = "library/" + import->path->value + ".acorn";

        Lexer lexer(filename);
        Parser parser(lexer);

        auto module2 = parser.parse(filename);

        if (lexer.has_errors() || parser.has_errors()) {
            continue;
        }

        for (auto statement : module2->code->statements) {
            module->code->statements.push_back(statement);
        }

        delete module2;
    }

    // read the remaining statements of the file
    while (!is_token(Token::EndOfFile)) {
        auto statement = readStatement();
        if (statement == nullptr) break;

        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    std::cerr << line << std::endl;
}

bool Parser::next_token() {
    bool found = false;
    Token token;

    // skip newline tokens
    while (m_lexer.next_token(token)) {
        if (token.kind != Token::Newline) {
            found = true;
            break;
        }
    }

    if (found) {
        m_tokens.push_back(token);
        return true;
    } else {
        return false;
    }
}

bool Parser::fill_token() {
    if (m_tokens.empty()) {
        if (!next_token()) {
            return false;
        }
    }

    return true;
}

bool Parser::read_token(Token::Kind kind, Token &token) {
    if (!fill_token()) {
        return false;
    }

    auto next_token = m_tokens.front();
    m_tokens.pop_front();

    if (next_token.kind != kind) {
        report(SyntaxError(next_token, kind));
        return false;
    }

    token = next_token;
    return true;
}

bool Parser::skip_token(Token::Kind kind) {
    Token tmp_token;
    return read_token(kind, tmp_token);
}

bool Parser::is_token(Token::Kind kind) {
    if (!fill_token()) {
        return false;
    }

    auto token = m_tokens.front();
    return token.kind == kind;
}

CodeBlock *Parser::readCodeBlock(bool in_switch) {
    debug("Reading CodeBlock...");

    return_if_false(fill_token())

    auto code = new CodeBlock(m_tokens.front());

    while (!is_token(Token::Deindent) && !(in_switch && (is_token(Token::CaseKeyword) || is_token(Token::DefaultKeyword)))) {
        auto statement = readStatement();
        if (statement == nullptr) {
            break;
        }

        code->statements.push_back(statement);
    }

    if (in_switch) {
        if (is_token(Token::CaseKeyword) || is_token(Token::DefaultKeyword) || is_token(Token::Deindent)) {
            return code;
        } else {
            assert(false);
        }
    } else {
        skip_token(Token::Deindent);
    }

    return code;
}

Expression *Parser::readExpression(bool parse_comma) {
    debug("Reading expression...");

    auto expr = readUnaryExpression(parse_comma);
    return_if_null(expr);

    if (is_token(Token::Operator) || is_token(Token::Assignment)) {
        return readBinaryExpression(expr, 0);
    } else {
        return expr;
    }
}

Identifier *Parser::readIdentifier(bool accept_parameters) {
    return_if_false(read_token(Token::Name, token));

    auto identifier = new Identifier(token, token.lexeme);

    if (accept_parameters && is_token(Token::OpenBrace)) {
        skip_token(Token::OpenBrace);

        while (!is_token(Token::CloseBrace)) {
            identifier->parameters.push_back(readIdentifier(true));

            if (is_token(Token::Comma)) {
                skip_token(Token::Comma);
            } else {
                break;
            }
        }

        skip_token(Token::CloseBrace);
    }

    return identifier;
}

Identifier *Parser::readOperator(bool accept_parameters) {
    return_if_false(read_token(Token::Operator, token));

    auto identifier = new Identifier(token, token.lexeme);

    if (accept_parameters && is_token(Token::OpenBrace)) {
        skip_token(Token::OpenBrace);

        while (!is_token(Token::CloseBrace)) {
            identifier->parameters.push_back(readIdentifier(true));

            if (is_token(Token::Comma)) {
                skip_token(Token::Comma);
            } else {
                break;
            }
        }

        skip_token(Token::CloseBrace);
    }

    return identifier;
}

ast::VariableDeclaration *Parser::readVariableDeclaration() {
    return_if_false(read_token(Token::LetKeyword, token));

    auto name = readIdentifier(false);
    return_if_null(name);

    ast::Identifier *type = nullptr;
    if (is_token(Token::AsKeyword)) {
        skip_token(Token::AsKeyword);

        type = readIdentifier(true);
        return_if_null(type);
    }

    return new VariableDeclaration(token, name, type);
}

IntegerLiteral *Parser::readIntegerLiteral() {
    return_if_false(read_token(Token::IntegerLiteral, token));

    IntegerLiteral *literal = new IntegerLiteral(token);

    std::stringstream ss;
    ss << token.lexeme;
    ss >> literal->value;

    return literal;
}

FloatLiteral *Parser::readFloatLiteral() {
    return_if_false(read_token(Token::FloatLiteral, token));

    FloatLiteral *literal = new FloatLiteral(token);
    literal->value = token.lexeme;

    return literal;
}

StringLiteral *Parser::readStringLiteral() {
    return_if_false(read_token(Token::StringLiteral, token));

    StringLiteral *literal = new StringLiteral(token);
    literal->value = token.lexeme.substr(1, token.lexeme.length() - 2);

    return literal;
}

SequenceLiteral *Parser::readSequenceLiteral() {
    return_if_false(read_token(Token::OpenBracket, token));

    SequenceLiteral *literal = new SequenceLiteral(token);

    while (!is_token(Token::CloseBracket)) {
        literal->elements.push_back(readExpression(false));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseBracket);

    return literal;
}

MappingLiteral *Parser::readMappingLiteral() {
    return_if_false(read_token(Token::OpenBrace, token));

    MappingLiteral *literal = new MappingLiteral(token);

    while (!is_token(Token::CloseBrace)) {
        Expression *key = readExpression(false);
        skip_token(Token::Colon);
        Expression *value = readExpression(false);

        literal->keys.push_back(key);
        literal->values.push_back(value);

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseBrace);

    return literal;
}

RecordLiteral *Parser::readRecordLiteral() {
    return_if_false(read_token(Token::NewKeyword, token));

    auto literal = new RecordLiteral(token);

    literal->name = readIdentifier(true);

    skip_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        literal->field_names.push_back(readIdentifier(false));
        skip_token(Token::Colon);
        literal->field_values.push_back(readExpression(false));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseParenthesis);

    return literal;
}

Call *Parser::readCall(Expression *operand) {
    return_if_false(read_token(Token::OpenParenthesis, token));

    Call *call = new Call(token);
    call->operand = operand;

    while (!is_token(Token::CloseParenthesis)) {
        call->arguments.push_back(readExpression(false));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseParenthesis);

    return call;
}

CCall *Parser::readCCall() {
    return_if_false(read_token(Token::CCallKeyword, token));

    CCall *ccall = new CCall(token);
    ccall->name = readIdentifier(false);

    return_if_false(skip_token(Token::OpenParenthesis));

    while (!is_token(Token::CloseParenthesis)) {
        ccall->parameters.push_back(readIdentifier(true));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseParenthesis);

    return_if_false(skip_token(Token::AsKeyword));
    ccall->returnType = readIdentifier(true);

    if (is_token(Token::UsingKeyword)) {
        skip_token(Token::UsingKeyword);

        while (true) {
            ccall->arguments.push_back(readExpression(false));

            if (is_token(Token::Comma)) {
                skip_token(Token::Comma);
            } else {
                break;
            }
        }
    }

    return ccall;
}

Cast *Parser::readCast(Expression *operand) {
    return_if_false(read_token(Token::AsKeyword, token));

    Cast *cast = new Cast(token);
    cast->operand = operand;
    cast->new_type = readIdentifier(true);
    return cast;
}

Selector *Parser::readSelector(Expression *operand) {
    return_if_false(read_token(Token::Dot, token));

    Identifier *name = nullptr;

    if (is_token(Token::IntegerLiteral)) {
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
    return_if_false(read_token(Token::OpenBracket, token));

    auto call = new Call(token);
    call->arguments.push_back(operand);
    call->arguments.push_back(readExpression(true));

    skip_token(Token::CloseBracket);

    if (is_token(Token::Assignment)) {
        skip_token(Token::Assignment);
        call->arguments.push_back(readExpression(true));
        call->operand = new Identifier(token, "setindex");
    } else {
        call->operand = new Identifier(token, "getindex");
    }

    return call;
}

While *Parser::readWhile() {
    return_if_false(read_token(Token::WhileKeyword, token));

    auto condition = readExpression(true);
    return_if_null(condition);

    return_if_false(skip_token(Token::Newline));

    auto code = readCodeBlock();
    return_if_null(code);

    return new While(token, condition, code);
}

CodeBlock *Parser::readFor() {
    /*
     * for item in blah
     *
     * let iterator = iterate(blah)
     * while True
     *     let maybe_item = next(iterator)
     *     if item = maybe_item
     *         <code>
     *     else
     *         break
     */

    return_if_false(read_token(Token::ForKeyword, token));

    auto variable = readIdentifier(false);
    return_if_null(variable);

    return_if_false(skip_token(Token::InKeyword));

    auto iterator = readExpression(true);
    return_if_null(iterator);

    return_if_false(skip_token(Token::Newline));

    auto loop_code = readCodeBlock();
    return_if_null(loop_code);

    std::string for_id;
    std::stringstream ss;
    ss << &token;
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
    return_if_false(read_token(Token::IfKeyword, token));

    If *expression = new If(token);

    if (is_token(Token::LetKeyword)) {
        auto lhs = readVariableDeclaration();
        return_if_null(lhs);

        Token assignment_token;
        return_if_false(read_token(Token::Assignment, assignment_token));

        auto rhs = readExpression(true);
        return_if_null(rhs);

        expression->condition = new Assignment(assignment_token, lhs, rhs);
    } else {
        expression->condition = readExpression(true);
        return_if_null(expression->condition);
    }

    return_if_false(skip_token(Token::Newline));

    expression->trueCode = new CodeBlock(m_tokens.front());
    expression->falseCode = nullptr;

    while (!is_token(Token::ElseKeyword) && !is_token(Token::Deindent)) {
        auto statement = readStatement();
        if (statement == nullptr) {
            break;
        }

        expression->trueCode->statements.push_back(statement);
    }

    if (is_token(Token::ElseKeyword)) {
        skip_token(Token::ElseKeyword);

        if (is_token(Token::IfKeyword)) {
            expression->falseCode = new CodeBlock(m_tokens.front());
            If *if_expr = readIf();
            expression->falseCode->statements.push_back(new ExpressionStatement(if_expr));
        } else {
            return_if_false(skip_token(Token::Newline));
            expression->falseCode = readCodeBlock();
        }
    } else {
        skip_token(Token::Deindent);
    }

    return expression;
}

Return *Parser::readReturn() {
    return_if_false(read_token(Token::ReturnKeyword, token));

    Return *r = new Return(token);
    r->expression = readExpression(true);
    return r;
}

Spawn *Parser::readSpawn() {
    return_if_false(read_token(Token::SpawnKeyword, token));

    Expression *expr = readExpression(true);
    Call *call = dynamic_cast<Call *>(expr);
    if (call) {
        return new Spawn(token, call);
    } else {
        report(SyntaxError(expr->token, "function call"));
        return nullptr;
    }
}

Case *Parser::readCase() {
    return_if_false(read_token(Token::CaseKeyword, token));

    auto condition = readExpression(true);
    return_if_null(condition);

    Expression *assignment = nullptr;
    if (is_token(Token::UsingKeyword)) {
        skip_token(Token::UsingKeyword);

        if (is_token(Token::LetKeyword)) {
            assignment = readVariableDeclaration();
        } else {
            assignment = readExpression(true);
        }
    }

    return_if_false(skip_token(Token::Newline));

    auto code = readCodeBlock(true);
    return_if_null(code);

    return new Case(token, condition, assignment, code);
}

Switch *Parser::readSwitch() {
    return_if_false(read_token(Token::SwitchKeyword, token));

    auto expression = readExpression(true);
    return_if_null(expression);

    return_if_false(skip_token(Token::Newline));

    std::vector<Case *> cases;
    while (is_token(Token::CaseKeyword)) {
        auto entry = readCase();
        return_if_null(entry);
        cases.push_back(entry);
    }

    CodeBlock *default_block = nullptr;
    if (is_token(Token::DefaultKeyword)) {
        default_block = readCodeBlock(true);
    }

    return_if_false(skip_token(Token::Deindent));

    return new Switch(token, expression, cases, default_block);
}

Expression *Parser::readUnaryExpression(bool parse_comma) {
    if (is_token(Token::Operator)) {
        Call *expr = new Call(m_tokens.front());
        expr->operand = readOperator(true);
        expr->arguments.push_back(readUnaryExpression(false));

        return expr;
    } else {
        return readOperandExpression(parse_comma);
    }
}

Expression *Parser::readBinaryExpression(Expression *lhs, int minPrecedence) {
    while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operatorPrecendence[m_tokens.front().lexeme] >= minPrecedence) {
        auto token = m_tokens.front();

        std::string opName = "=";

        Identifier *op = 0;
        if (is_token(Token::Operator)) {
            op = readOperator(true);
            opName = op->value;
        } else {
            skip_token(Token::Assignment);
        }

        Expression *rhs = readOperandExpression(true);

        while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operatorPrecendence[m_tokens.front().lexeme] > m_operatorPrecendence[opName]) {
            rhs = readBinaryExpression(rhs, m_operatorPrecendence[m_tokens.front().lexeme]);
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
    if (is_token(Token::OpenParenthesis)) {
        skip_token(Token::OpenParenthesis);
        Expression *expr = readExpression(true);
        return_if_false(skip_token(Token::CloseParenthesis));
        return expr;
    } else if (is_token(Token::IntegerLiteral)) {
        return readIntegerLiteral();
    } else if (is_token(Token::FloatLiteral)) {
        return readFloatLiteral();
    } else if (is_token(Token::StringLiteral)) {
        return readStringLiteral();
    } else if (is_token(Token::OpenBracket)) {
        return readSequenceLiteral();
    } else if (is_token(Token::OpenBrace)) {
        return readMappingLiteral();
    } else if (is_token(Token::WhileKeyword)) {
        return readWhile();
    } else if (is_token(Token::ForKeyword)) {
        return readFor();
    } else if (is_token(Token::IfKeyword)) {
        return readIf();
    } else if (is_token(Token::SwitchKeyword)) {
        return readSwitch();
    } else if (is_token(Token::ReturnKeyword)) {
        return readReturn();
    } else if (is_token(Token::SpawnKeyword)) {
        return readSpawn();
    } else if (is_token(Token::CCallKeyword)) {
        return readCCall();
    } else if (is_token(Token::NewKeyword)) {
        return readRecordLiteral();
    } else if (is_token(Token::Name)) {
        return readIdentifier(true);
    } else if (!m_tokens.empty()) {  // FIXME refactor into function
        report(SyntaxError(m_tokens.front(), "primary expression"));
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
        if (is_token(Token::OpenParenthesis)) {
            left = readCall(left);
        } else if (is_token(Token::OpenBracket)) {
            left = readIndex(left);
        } else if (is_token(Token::AsKeyword)) {
            left = readCast(left);
        } else if (is_token(Token::Dot)) {
            left = readSelector(left);
        } else if (parse_comma && is_token(Token::Comma)) {
            skip_token(Token::Comma);

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

    if (is_token(Token::InoutKeyword)) {
        skip_token(Token::InoutKeyword);
        parameter->inout = true;
    } else {
        parameter->inout = false;
    }

    parameter->name = readIdentifier(false);

    return_if_false(skip_token(Token::Colon));

    parameter->typeNode = readIdentifier(true);

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    auto lhs = readVariableDeclaration();
    return_if_null(lhs);

    return_if_false(read_token(Token::Assignment, token));

    auto rhs = readExpression(true);
    return_if_null(rhs);

    auto definition = new VariableDefinition(lhs->token);
    definition->name = lhs->name();
    definition->assignment = new Assignment(token, lhs, rhs);
    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    return_if_false(read_token(Token::DefKeyword, token));

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (is_token(Token::Name)) {
        definition->name = readIdentifier(true);
    } else if (is_token(Token::Operator)) {
        definition->name = readOperator(true);
    } else {
        report(SyntaxError(m_tokens.front(), "identifier or operator"));
        return nullptr;
    }

    debug("Name: " + definition->name->value);

    skip_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        auto p = readParameter();
        return_if_null(p);

        definition->parameters.push_back(p);

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;  // no more parameters, apparently
        }
    }

    return_if_false(skip_token(Token::CloseParenthesis));
    return_if_false(skip_token(Token::Arrow));

    definition->returnType = readIdentifier(true);
    return_if_null(definition->returnType)

    return_if_false(skip_token(Token::Colon));
    return_if_false(skip_token(Token::Indent));

    definition->code = readCodeBlock();
    return_if_null(definition->code)

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    return_if_false(read_token(Token::TypeKeyword, token));

    TypeDefinition *definition = new TypeDefinition(token);

    definition->name = readIdentifier(true);

    if (is_token(Token::AsKeyword)) {
        skip_token(Token::AsKeyword);
        definition->alias = readIdentifier(true);
    } else {
        return_if_false(skip_token(Token::Newline));

        while (!is_token(Token::Deindent)) {
            definition->field_names.push_back(readIdentifier(false));
            skip_token(Token::AsKeyword);
            definition->field_types.push_back(readIdentifier(true));
            return_if_false(skip_token(Token::Newline));
        }

        skip_token(Token::Deindent);
    }

    return definition;
}

Statement *Parser::readStatement() {
    debug("Reading Statement...");

    Definition *def = nullptr;

    if (is_token(Token::LetKeyword)) {
        def = readVariableDefinition();
    } else if (is_token(Token::DefKeyword)) {
        def = readFunctionDefinition();
    } else if (is_token(Token::TypeKeyword)) {
        def = readTypeDefinition();
    } else {
        auto expression = readExpression(true);
        return_if_null(expression);
        return new ExpressionStatement(expression);
    }

    if (def != nullptr) {
        return new DefinitionStatement(def);
    } else {
        return nullptr;
    }
}

ImportStatement *Parser::readImportStatement() {
    return_if_false(read_token(Token::ImportKeyword, token));
    StringLiteral *path = readStringLiteral();
    return_if_false(skip_token(Token::Newline));

    return new ImportStatement(token, path);
}
