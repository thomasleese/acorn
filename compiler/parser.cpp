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
    m_operator_precendence["+"] = 1;
    m_operator_precendence["-"] = 1;
}

Parser::~Parser() {

}

SourceFile *Parser::parse(std::string name) {
    return_if_false(fill_token())

    auto module = new SourceFile(front_token(), name);

    // read import expressions, which must appear at the top of a source file
    while (is_token(Token::ImportKeyword)) {
        module->imports.push_back(read_import_expression());
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

        for (auto expression : module2->code->expressions()) {
            module->code->add_expression(expression);
        }

        delete module2;
    }

    // read the remaining expressions of the file
    while (!is_token(Token::EndOfFile)) {
        auto expression = read_expression();
        if (expression == nullptr) break;

        module->code->add_expression(expression);
    }

    return module;
}

void Parser::debug(std::string line) {
    std::cerr << line << std::endl;
}

Token Parser::front_token() {
    return m_tokens.front();
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

    auto next_token = front_token();
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

bool Parser::is_token_available() {
    if (!fill_token()) {
        return false;
    }

    return !m_tokens.empty();
}

bool Parser::is_token(Token::Kind kind) {
    if (!is_token_available()) {
        return false;
    }

    return front_token().kind == kind;
}

Block *Parser::read_block(bool in_switch) {
    debug("Reading Block...");

    return_if_false(fill_token())

    auto code = new Block(front_token());

    while (!is_token(Token::Deindent) && !(in_switch && (is_token(Token::CaseKeyword) || is_token(Token::DefaultKeyword)))) {
        auto expression = read_expression();
        if (expression == nullptr) {
            break;
        }

        code->add_expression(expression);
    }

    if (in_switch) {
        if (is_token(Token::CaseKeyword) || is_token(Token::DefaultKeyword) || is_token(Token::Deindent)) {
            return code;
        } else {
            assert(false);
        }
    } else {
        skip_token(Token::Deindent);
        skip_token(Token::EndKeyword);
    }

    return code;
}

Expression *Parser::read_expression(bool parse_comma) {
    debug("Reading expression...");

    auto expr = read_unary_expression(parse_comma);
    return_if_null(expr);

    if (is_token(Token::Operator) || is_token(Token::Assignment)) {
        return read_binary_expression(expr, 0);
    } else {
        return expr;
    }
}

Name *Parser::read_name(bool accept_parameters) {
    return_if_false(read_token(Token::Name, token));

    auto identifier = new Name(token, token.lexeme);

    if (accept_parameters && is_token(Token::OpenBrace)) {
        skip_token(Token::OpenBrace);

        while (!is_token(Token::CloseBrace)) {
            identifier->add_parameter(read_name(true));

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

Name *Parser::read_operator(bool accept_parameters) {
    return_if_false(read_token(Token::Operator, token));

    auto identifier = new Name(token, token.lexeme);

    if (accept_parameters && is_token(Token::OpenBrace)) {
        skip_token(Token::OpenBrace);

        while (!is_token(Token::CloseBrace)) {
            identifier->add_parameter(read_name(true));

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

ast::VariableDeclaration *Parser::read_variable_declaration() {
    return_if_false(read_token(Token::LetKeyword, token));

    auto name = read_name(false);
    return_if_null(name);

    ast::Name *type = nullptr;
    if (is_token(Token::AsKeyword)) {
        skip_token(Token::AsKeyword);

        type = read_name(true);
        return_if_null(type);
    }

    return new VariableDeclaration(token, name, type);
}

IntegerLiteral *Parser::read_integer_literal() {
    return_if_false(read_token(Token::IntegerLiteral, token));

    std::stringstream ss;
    ss << token.lexeme;

    return new IntegerLiteral(token, ss.str());
}

FloatLiteral *Parser::read_float_literal() {
    return_if_false(read_token(Token::FloatLiteral, token));

    FloatLiteral *literal = new FloatLiteral(token);
    literal->value = token.lexeme;

    return literal;
}

StringLiteral *Parser::read_string_literal() {
    return_if_false(read_token(Token::StringLiteral, token));

    StringLiteral *literal = new StringLiteral(token);
    literal->value = token.lexeme.substr(1, token.lexeme.length() - 2);

    return literal;
}

SequenceLiteral *Parser::read_sequence_literal() {
    return_if_false(read_token(Token::OpenBracket, token));

    SequenceLiteral *literal = new SequenceLiteral(token);

    while (!is_token(Token::CloseBracket)) {
        literal->elements.push_back(read_expression(false));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseBracket);

    return literal;
}

MappingLiteral *Parser::read_mapping_literal() {
    return_if_false(read_token(Token::OpenBrace, token));

    MappingLiteral *literal = new MappingLiteral(token);

    while (!is_token(Token::CloseBrace)) {
        Expression *key = read_expression(false);
        skip_token(Token::Colon);
        Expression *value = read_expression(false);

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

RecordLiteral *Parser::read_record_literal() {
    return_if_false(read_token(Token::NewKeyword, token));

    auto literal = new RecordLiteral(token);

    literal->name = read_name(true);

    skip_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        literal->field_names.push_back(read_name(false));
        skip_token(Token::Colon);
        literal->field_values.push_back(read_expression(false));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseParenthesis);

    return literal;
}

Call *Parser::read_call(Expression *operand) {
    return_if_false(read_token(Token::OpenParenthesis, token));

    Call *call = new Call(token);
    call->operand = operand;

    while (!is_token(Token::CloseParenthesis)) {
        call->arguments.push_back(read_expression(false));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseParenthesis);

    return call;
}

CCall *Parser::read_ccall() {
    return_if_false(read_token(Token::CCallKeyword, token));

    CCall *ccall = new CCall(token);
    ccall->name = read_name(false);

    return_if_false(skip_token(Token::OpenParenthesis));

    while (!is_token(Token::CloseParenthesis)) {
        ccall->parameters.push_back(read_name(true));

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;
        }
    }

    skip_token(Token::CloseParenthesis);

    return_if_false(skip_token(Token::AsKeyword));
    ccall->given_return_type = read_name(true);

    if (is_token(Token::UsingKeyword)) {
        skip_token(Token::UsingKeyword);

        while (true) {
            ccall->arguments.push_back(read_expression(false));

            if (is_token(Token::Comma)) {
                skip_token(Token::Comma);
            } else {
                break;
            }
        }
    }

    return ccall;
}

Cast *Parser::read_cast(Expression *operand) {
    return_if_false(read_token(Token::AsKeyword, token));

    Cast *cast = new Cast(token);
    cast->operand = operand;
    cast->new_type = read_name(true);
    return cast;
}

Selector *Parser::read_selector(Expression *operand) {
    return_if_false(read_token(Token::Dot, token));

    Name *name = nullptr;

    if (is_token(Token::IntegerLiteral)) {
        auto il = read_integer_literal();
        name = new Name(il->token(), il->value());
        delete il;
    } else {
        name = read_name(true);
    }

    return_if_null(name);

    return new Selector(token, operand, name);
}

Call *Parser::read_index(Expression *operand) {
    return_if_false(read_token(Token::OpenBracket, token));

    auto call = new Call(token);
    call->arguments.push_back(operand);
    call->arguments.push_back(read_expression(true));

    skip_token(Token::CloseBracket);

    if (is_token(Token::Assignment)) {
        skip_token(Token::Assignment);
        call->arguments.push_back(read_expression(true));
        call->operand = new Name(token, "setindex");
    } else {
        call->operand = new Name(token, "getindex");
    }

    return call;
}

While *Parser::read_while() {
    return_if_false(read_token(Token::WhileKeyword, token));

    auto condition = read_expression(true);
    return_if_null(condition);

    return_if_false(skip_token(Token::Newline));
    return_if_false(skip_token(Token::Indent));

    auto body = read_expression();
    return_if_null(body);

    return_if_false(skip_token(Token::Deindent));
    return_if_false(skip_token(Token::EndKeyword));

    return new While(token, condition, body);
}

Block *Parser::read_for() {
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

    auto variable = read_name(false);
    return_if_null(variable);

    return_if_false(skip_token(Token::InKeyword));

    auto iterator = read_expression(true);
    return_if_null(iterator);

    return_if_false(skip_token(Token::Newline));

    auto loop_code = read_block();
    return_if_null(loop_code);

    std::string for_id;
    std::stringstream ss;
    ss << &token;
    ss >> for_id;

    std::string state_variable_name = "state_" + for_id;
    std::string next_state_variable_name = "next_state_" + for_id;

    auto code_block = new Block(token);

    auto state_variable = new VariableDefinition(token, state_variable_name, new Call(token, "start", iterator));
    code_block->add_expression(state_variable);

    auto condition = new Call(token, "not", new Call(token, "done", iterator, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name()));
    auto while_code = new While(token, condition, loop_code);

    auto next_state_variable = new VariableDefinition(token, next_state_variable_name, new Call(token, "next", iterator, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name()));
    loop_code->insert_expression(0, next_state_variable);
    loop_code->insert_expression(1, new Assignment(token, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs), new Selector(token, static_cast<ast::VariableDeclaration *>(next_state_variable->assignment->lhs)->name(), "1")));
    loop_code->insert_expression(1, new Assignment(token, new VariableDeclaration(token, new Name(token, next_state_variable_name)), new Selector(token, static_cast<ast::VariableDeclaration *>(next_state_variable->assignment->lhs)->name(), "0")));

    code_block->add_expression(while_code);

    return code_block;
}

If *Parser::read_if() {
    debug("Reading if");

    return_if_false(read_token(Token::IfKeyword, token));

    auto expression = new If(token);

    if (is_token(Token::LetKeyword)) {
        auto lhs = read_variable_declaration();
        return_if_null(lhs);

        Token assignment_token;
        return_if_false(read_token(Token::Assignment, assignment_token));

        auto rhs = read_expression(true);
        return_if_null(rhs);

        expression->condition = new Assignment(assignment_token, lhs, rhs);
    } else {
        expression->condition = read_expression();
        return_if_null(expression->condition);
    }

    debug("Reading indent");

    return_if_false(skip_token(Token::Indent));

    expression->true_case = read_expression();
    expression->false_case = nullptr;

    return_if_false(skip_token(Token::Deindent));

    debug("Reading potential false case");

    if (is_token(Token::ElseKeyword)) {
        skip_token(Token::ElseKeyword);

        if (is_token(Token::IfKeyword)) {
            expression->false_case = read_if();
        } else {
            return_if_false(skip_token(Token::Indent));
            expression->false_case = read_expression();
            return_if_false(skip_token(Token::Deindent));
            return_if_false(skip_token(Token::EndKeyword));
        }
    } else {
        return_if_false(skip_token(Token::Deindent));
        return_if_false(skip_token(Token::EndKeyword));
    }

    return expression;
}

Return *Parser::read_return() {
    return_if_false(read_token(Token::ReturnKeyword, token));

    Return *r = new Return(token);
    r->expression = read_expression();
    return r;
}

Spawn *Parser::read_spawn() {
    return_if_false(read_token(Token::SpawnKeyword, token));

    Expression *expr = read_expression();
    Call *call = dynamic_cast<Call *>(expr);
    if (call) {
        return new Spawn(token, call);
    } else {
        report(SyntaxError(expr->token(), "function call"));
        return nullptr;
    }
}

Case *Parser::read_case() {
    return_if_false(read_token(Token::CaseKeyword, token));

    auto condition = read_expression();
    return_if_null(condition);

    Expression *assignment = nullptr;
    if (is_token(Token::UsingKeyword)) {
        skip_token(Token::UsingKeyword);

        if (is_token(Token::LetKeyword)) {
            assignment = read_variable_declaration();
        } else {
            assignment = read_expression(true);
        }
    }

    return_if_false(skip_token(Token::Newline));

    auto code = read_block(true);
    return_if_null(code);

    return new Case(token, condition, assignment, code);
}

Switch *Parser::read_switch() {
    return_if_false(read_token(Token::SwitchKeyword, token));

    auto expression = read_expression(true);
    return_if_null(expression);

    return_if_false(skip_token(Token::Newline));

    std::vector<Case *> cases;
    while (is_token(Token::CaseKeyword)) {
        auto entry = read_case();
        return_if_null(entry);
        cases.push_back(entry);
    }

    Block *default_block = nullptr;
    if (is_token(Token::DefaultKeyword)) {
        default_block = read_block(true);
    }

    return_if_false(skip_token(Token::Deindent));

    return new Switch(token, expression, cases, default_block);
}

Expression *Parser::read_unary_expression(bool parse_comma) {
    if (is_token(Token::Operator)) {
        Call *expr = new Call(front_token());
        expr->operand = read_operator(true);
        expr->arguments.push_back(read_unary_expression(false));

        return expr;
    } else {
        return read_operand_expression(parse_comma);
    }
}

Expression *Parser::read_binary_expression(Expression *lhs, int min_precedence) {
    while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operator_precendence[front_token().lexeme] >= min_precedence) {
        auto saved_token = front_token();

        std::string opName = "";

        Name *op = nullptr;
        if (is_token(Token::Operator)) {
            op = read_operator(true);
            opName = op->value();
        } else {
            skip_token(Token::Assignment);
        }

        Expression *rhs = read_operand_expression(true);

        while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operator_precendence[front_token().lexeme] > m_operator_precendence[opName]) {
            rhs = read_binary_expression(rhs, m_operator_precendence[front_token().lexeme]);
        }

        auto call = new Call(saved_token);
        call->operand = op;
        call->arguments.push_back(lhs);
        call->arguments.push_back(rhs);

        lhs = call;
    }

    return lhs;
}

Expression *Parser::read_parenthesis_expression() {
    return_if_false(skip_token(Token::OpenParenthesis));
    auto expression = read_expression(true);
    return_if_false(skip_token(Token::CloseParenthesis));
    return expression;
}

Expression *Parser::read_primary_expression() {
    debug("Reading primary expression");

    if (is_token(Token::OpenParenthesis)) {
        return read_parenthesis_expression();
    } else if (is_token(Token::IntegerLiteral)) {
        return read_integer_literal();
    } else if (is_token(Token::FloatLiteral)) {
        return read_float_literal();
    } else if (is_token(Token::StringLiteral)) {
        return read_string_literal();
    } else if (is_token(Token::OpenBracket)) {
        return read_sequence_literal();
    } else if (is_token(Token::OpenBrace)) {
        return read_mapping_literal();
    } else if (is_token(Token::WhileKeyword)) {
        return read_while();
    } else if (is_token(Token::ForKeyword)) {
        return read_for();
    } else if (is_token(Token::IfKeyword)) {
        return read_if();
    } else if (is_token(Token::SwitchKeyword)) {
        return read_switch();
    } else if (is_token(Token::ReturnKeyword)) {
        return read_return();
    } else if (is_token(Token::SpawnKeyword)) {
        return read_spawn();
    } else if (is_token(Token::CCallKeyword)) {
        return read_ccall();
    } else if (is_token(Token::NewKeyword)) {
        return read_record_literal();
    } else if (is_token(Token::Name)) {
        return read_name(true);
    } else if (!is_token_available()) {  // FIXME refactor into function
        report(SyntaxError(front_token(), "primary expression"));
    }

    return nullptr;
}

Expression *Parser::read_operand_expression(bool parse_comma) {
    auto left = read_primary_expression();
    return_if_null(left);

    while (true) {
        if (is_token(Token::OpenParenthesis)) {
            left = read_call(left);
        } else if (is_token(Token::OpenBracket)) {
            left = read_index(left);
        } else if (is_token(Token::AsKeyword)) {
            left = read_cast(left);
        } else if (is_token(Token::Dot)) {
            left = read_selector(left);
        } else if (parse_comma && is_token(Token::Comma)) {
            skip_token(Token::Comma);

            auto rhs = read_expression(true);
            std::vector<Expression *> elements;

            elements.push_back(left);

            if (auto tuple = dynamic_cast<TupleLiteral *>(rhs)) {
                for (auto e : tuple->elements()) {
                    elements.push_back(e);
                }
            } else {
                elements.push_back(rhs);
            }

            left = new TupleLiteral(left->token(), elements);
        } else {
            break;
        }
    }

    return left;
}

Parameter *Parser::read_parameter() {
    auto parameter = new Parameter(front_token());

    if (is_token(Token::InoutKeyword)) {
        skip_token(Token::InoutKeyword);
        parameter->inout = true;
    } else {
        parameter->inout = false;
    }

    parameter->name = read_name(false);

    return_if_false(skip_token(Token::AsKeyword));

    parameter->typeNode = read_name(true);

    return parameter;
}

VariableDefinition *Parser::read_variable_definition() {
    auto lhs = read_variable_declaration();
    return_if_null(lhs);

    return_if_false(read_token(Token::Assignment, token));

    auto rhs = read_expression(true);
    return_if_null(rhs);

    return_if_false(skip_token(Token::Indent));

    auto body = read_expression();

    return_if_false(skip_token(Token::Deindent));
    return_if_false(skip_token(Token::EndKeyword));

    auto definition = new VariableDefinition(lhs->token());
    definition->assignment = new Assignment(token, lhs, rhs);
    definition->set_body(body);
    return definition;
}

FunctionDefinition *Parser::read_function_definition() {
    debug("Reading FunctionDefinition...");

    return_if_false(read_token(Token::DefKeyword, token));

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (is_token(Token::Name)) {
        definition->set_name(read_name(true));
    } else if (is_token(Token::Operator)) {
        definition->set_name(read_operator(true));
    } else {
        report(SyntaxError(front_token(), "identifier or operator"));
        return nullptr;
    }

    debug("Name: " + definition->name()->value());

    skip_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        auto p = read_parameter();
        return_if_null(p);

        definition->parameters.push_back(p);

        if (is_token(Token::Comma)) {
            skip_token(Token::Comma);
        } else {
            break;  // no more parameters, apparently
        }
    }

    return_if_false(skip_token(Token::CloseParenthesis));

    if (is_token(Token::AsKeyword)) {
        return_if_false(skip_token(Token::AsKeyword));
        definition->given_return_type = read_name(true);
        return_if_null(definition->given_return_type);
    }

    return_if_false(skip_token(Token::Indent));

    definition->body = read_expression();
    return_if_null(definition->body);

    return_if_false(skip_token(Token::Deindent));
    return_if_false(skip_token(Token::EndKeyword));

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::read_type_definition() {
    return_if_false(read_token(Token::TypeKeyword, token));

    TypeDefinition *definition = new TypeDefinition(token);

    definition->set_name(read_name(true));

    if (is_token(Token::AsKeyword)) {
        skip_token(Token::AsKeyword);
        definition->alias = read_name(true);
    } else {
        return_if_false(skip_token(Token::Newline));

        while (!is_token(Token::Deindent)) {
            definition->field_names.push_back(read_name(false));
            skip_token(Token::AsKeyword);
            definition->field_types.push_back(read_name(true));
            return_if_false(skip_token(Token::Newline));
        }

        skip_token(Token::Deindent);
    }

    return definition;
}

Expression *Parser::read_expression() {
    debug("Reading Expression...");

    if (is_token(Token::LetKeyword)) {
        return read_variable_definition();
    } else if (is_token(Token::DefKeyword)) {
        return read_function_definition();
    } else if (is_token(Token::TypeKeyword)) {
        return read_type_definition();
    } else {
        return read_expression(true);
    }
}

Import *Parser::read_import_expression() {
    return_if_false(read_token(Token::ImportKeyword, token));
    StringLiteral *path = read_string_literal();
    return_if_false(skip_token(Token::Newline));

    return new Import(token, path);
}
