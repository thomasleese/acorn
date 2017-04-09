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

Parser::Parser(Lexer &lexer) : m_lexer(lexer) {
    m_operator_precendence["+"] = 1;
    m_operator_precendence["-"] = 1;
}

Parser::~Parser() {

}

SourceFile *Parser::parse(std::string name) {
    return_if_false(fill_token());

    auto module = new SourceFile(front_token(), name);

    while (is_token(Token::ImportKeyword)) {
        auto import = read_import_expression();
        return_if_null(import);

        std::string filename = "../library/" + import->path->value() + ".acorn";

        Lexer lexer(filename);
        Parser parser(lexer);

        auto imported_module = parser.parse(filename);

        if (lexer.has_errors() || parser.has_errors()) {
            continue;
        }

        for (auto expression : imported_module->code->expressions()) {
            module->code->add_expression(expression);
        }

        delete imported_module;
        delete import;
    }

    while (!is_token(Token::EndOfFile)) {
        auto expression = read_expression();
        return_if_null(expression);
        module->code->add_expression(expression);
    }

    return module;
}

void Parser::debug(std::string line) {
    std::cerr << line << std::endl;
}

Token Parser::front_token() {
    fill_token();
    return m_tokens.front();
}

bool Parser::next_non_newline_token() {
    bool found = false;
    Token token;

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

void Parser::collapse_deindent_indent_tokens() {
    while (m_tokens.size() >= 2 && m_tokens[0].kind == Token::Deindent && m_tokens[1].kind == Token::Indent) {
        m_tokens.erase(m_tokens.begin());
        m_tokens.erase(m_tokens.begin());
    }
}

bool Parser::next_token() {
    while (m_tokens.size() < 2) {
        if (next_non_newline_token() && next_non_newline_token()) {
            collapse_deindent_indent_tokens();
        } else {
            return false;
        }
    }

    return true;
}

bool Parser::fill_token() {
    if (m_tokens.size() < 2 && !next_token()) {
        return false;
    }

    return true;
}

bool Parser::read_token(Token::Kind kind, Token &token) {
    if (!fill_token()) {
        return false;
    }

    auto next_front_token = front_token();
    m_tokens.pop_front();

    if (next_front_token.kind != kind) {
        report(SyntaxError(next_front_token, kind));
        return false;
    }

    token = next_front_token;
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

bool Parser::is_and_skip_token(Token::Kind kind) {
    return is_token(kind) && skip_token(kind);
}

bool Parser::skip_deindent_and_end_token() {
    return skip_token(Token::Deindent) && skip_token(Token::EndKeyword);
}

Block *Parser::read_block(bool read_end) {
    return_if_false(read_token(Token::Indent, token));

    auto block = new Block(token);

    while (!is_token(Token::Deindent)) {
        auto expression = read_expression();
        return_if_null(expression);
        block->add_expression(expression);
    }

    return_if_false(skip_token(Token::Deindent));

    if (read_end) {
        return_if_false(skip_token(Token::EndKeyword));
    }

    return block;
}

Expression *Parser::read_expression(bool parse_comma) {
    if (is_token(Token::LetKeyword)) {
        return read_let();
    } else if (is_token(Token::DefKeyword)) {
        return read_def();
    } else if (is_token(Token::TypeKeyword)) {
        return read_type();
    } else if (is_token(Token::ModuleKeyword)) {
        return read_module();
    } else {
        auto expr = read_unary_expression(parse_comma);
        return_if_null(expr);

        if (is_token(Token::Operator) || is_token(Token::Assignment)) {
            return read_binary_expression(expr, 0);
        } else {
            return expr;
        }
    }
}

Name *Parser::read_name_or_operator(Token::Kind kind, bool accept_parameters) {
    return_if_false(read_token(kind, token));

    auto identifier = new Name(token, token.lexeme);

    if (accept_parameters && is_and_skip_token(Token::OpenBrace)) {
        while (!is_token(Token::CloseBrace)) {
            identifier->add_parameter(read_name(true));
            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_if_false(skip_token(Token::CloseBrace));
    }

    return identifier;
}

Name *Parser::read_name(bool accept_parameters) {
    return read_name_or_operator(Token::Name, accept_parameters);
}

Name *Parser::read_operator(bool accept_parameters) {
    return read_name_or_operator(Token::Operator, accept_parameters);
}

ast::VariableDeclaration *Parser::read_variable_declaration() {
    return_if_false(read_token(Token::LetKeyword, token));

    auto name = read_name(false);
    return_if_null(name);

    ast::Name *type = nullptr;
    if (is_and_skip_token(Token::AsKeyword)) {
        type = read_name(true);
        return_if_null(type);
    }

    return new VariableDeclaration(token, name, type);
}

Int *Parser::read_int() {
    return_if_false(read_token(Token::Int, token));
    return new Int(token, token.lexeme);
}

Float *Parser::read_float() {
    return_if_false(read_token(Token::Float, token));
    return new Float(token, token.lexeme);
}

String *Parser::read_string() {
    return_if_false(read_token(Token::String, token));
    return new String(token, token.lexeme);
}

List *Parser::read_list() {
    return_if_false(read_token(Token::OpenBracket, token));

    auto literal = new List(token);

    while (!is_token(Token::CloseBracket)) {
        literal->elements.push_back(read_expression(false));
        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_if_false(skip_token(Token::CloseBracket));

    return literal;
}

Dictionary *Parser::read_dictionary() {
    return_if_false(read_token(Token::OpenBrace, token));

    auto literal = new Dictionary(token);

    while (!is_token(Token::CloseBrace)) {
        auto key = read_expression(false);
        return_if_false(skip_token(Token::Colon));
        auto value = read_expression(false);

        literal->keys.push_back(key);
        literal->values.push_back(value);

        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_if_false(skip_token(Token::CloseBrace));

    return literal;
}

Call *Parser::read_call(Expression *operand) {
    return_if_false(read_token(Token::OpenParenthesis, token));

    auto call = new Call(token, operand);

    while (!is_token(Token::CloseParenthesis)) {
        auto saved_token = front_token();

        ast::Name *name = nullptr;
        auto expression = read_expression();

        if (is_and_skip_token(Token::Colon)) {
            name = dynamic_cast<Name *>(expression);
            if (name == nullptr) {
                report(SyntaxError(saved_token, Token::Name));
                return nullptr;
            }

            expression = read_expression(false);
        }

        return_if_null(expression);

        if (name == nullptr) {
            call->add_positional_argument(expression);
        } else {
            call->add_keyword_argument(name->value(), expression);
            delete name;
        }

        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_if_false(skip_token(Token::CloseParenthesis));

    return call;
}

CCall *Parser::read_ccall() {
    return_if_false(read_token(Token::CCallKeyword, token));

    CCall *ccall = new CCall(token);
    ccall->name = read_name(false);

    return_if_false(skip_token(Token::OpenParenthesis));

    while (!is_token(Token::CloseParenthesis)) {
        ccall->parameters.push_back(read_name(true));
        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_if_false(skip_token(Token::CloseParenthesis));

    return_if_false(skip_token(Token::AsKeyword));
    ccall->given_return_type = read_name(true);

    if (is_and_skip_token(Token::UsingKeyword)) {
        while (true) {
            ccall->arguments.push_back(read_expression(false));
            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }
    }

    return ccall;
}

Cast *Parser::read_cast(Expression *operand) {
    return_if_false(read_token(Token::AsKeyword, token));

    auto new_type = read_name(true);
    return_if_null(new_type);

    return new Cast(token, operand, new_type);
}

Selector *Parser::read_selector(Expression *operand) {
    return_if_false(read_token(Token::Dot, token));

    Name *name = nullptr;

    if (is_token(Token::Int)) {
        auto il = read_int();
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
    call->add_positional_argument(operand);
    call->add_positional_argument(read_expression(true));

    return_if_false(skip_token(Token::CloseBracket));

    if (is_and_skip_token(Token::Assignment)) {
        call->add_positional_argument(read_expression(true));
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

    auto state_variable = new Let(token, state_variable_name, new Call(token, "start", iterator));
    code_block->add_expression(state_variable);

    auto condition = new Call(token, "not", new Call(token, "done", iterator, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name()));
    auto while_code = new While(token, condition, loop_code);

    auto next_state_variable = new Let(token, next_state_variable_name, new Call(token, "next", iterator, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs)->name()));
    loop_code->insert_expression(0, next_state_variable);
    loop_code->insert_expression(1, new Assignment(token, static_cast<ast::VariableDeclaration *>(state_variable->assignment->lhs), new Selector(token, static_cast<ast::VariableDeclaration *>(next_state_variable->assignment->lhs)->name(), "1")));
    loop_code->insert_expression(1, new Assignment(token, new VariableDeclaration(token, new Name(token, next_state_variable_name)), new Selector(token, static_cast<ast::VariableDeclaration *>(next_state_variable->assignment->lhs)->name(), "0")));

    code_block->add_expression(while_code);

    return code_block;
}

If *Parser::read_if() {
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

    return_if_false(skip_token(Token::Indent));

    expression->true_case = read_expression();
    expression->false_case = nullptr;

    return_if_false(skip_token(Token::Deindent));

    if (is_token(Token::ElseKeyword)) {
        skip_token(Token::ElseKeyword);

        if (is_token(Token::IfKeyword)) {
            expression->false_case = read_if();
        } else {
            return_if_false(skip_token(Token::Indent));
            expression->false_case = read_expression();
            return_if_false(skip_deindent_and_end_token());
        }
    } else {
        return_if_false(skip_deindent_and_end_token());
    }

    return expression;
}

Return *Parser::read_return() {
    return_if_false(read_token(Token::ReturnKeyword, token));

    auto expression = read_expression();
    return_if_null(expression);

    return new Return(token, expression);
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
    if (is_and_skip_token(Token::UsingKeyword)) {
        if (is_token(Token::LetKeyword)) {
            assignment = read_variable_declaration();
        } else {
            assignment = read_expression(true);
        }
    }

    return_if_null(assignment);
    return_if_false(skip_token(Token::Newline));

    auto code = read_block(false);
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
        default_block = read_block(false);
        return_if_null(default_block);
    }

    return_if_false(skip_token(Token::EndKeyword));

    return new Switch(token, expression, cases, default_block);
}

Expression *Parser::read_unary_expression(bool parse_comma) {
    if (is_token(Token::Operator)) {
        auto operand = read_operator(true);
        return_if_null(operand);

        auto call = new Call(front_token(), operand);
        call->add_positional_argument(read_unary_expression(false));
        return call;
    } else {
        return read_operand_expression(parse_comma);
    }
}

Expression *Parser::read_binary_expression(Expression *lhs, int min_precedence) {
    while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operator_precendence[front_token().lexeme] >= min_precedence) {
        auto saved_token = front_token();

        auto op = read_operator(true);
        return_if_null(op);

        auto rhs = read_operand_expression(true);

        while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operator_precendence[front_token().lexeme] > m_operator_precendence[op->value()]) {
            rhs = read_binary_expression(rhs, m_operator_precendence[front_token().lexeme]);
        }

        auto call = new Call(saved_token, op);
        call->add_positional_argument(lhs);
        call->add_positional_argument(rhs);

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
    if (is_token(Token::OpenParenthesis)) {
        return read_parenthesis_expression();
    } else if (is_token(Token::Int)) {
        return read_int();
    } else if (is_token(Token::Float)) {
        return read_float();
    } else if (is_token(Token::String)) {
        return read_string();
    } else if (is_token(Token::OpenBracket)) {
        return read_list();
    } else if (is_token(Token::OpenBrace)) {
        return read_dictionary();
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
    } else if (is_token(Token::Name)) {
        return read_name(true);
    } else {
        report(SyntaxError(front_token(), "primary expression"));
        return nullptr;
    }
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
        } else if (parse_comma && is_and_skip_token(Token::Comma)) {
            auto rhs = read_expression(true);
            std::vector<Expression *> elements;

            elements.push_back(left);

            if (auto tuple = dynamic_cast<Tuple *>(rhs)) {
                for (auto e : tuple->elements()) {
                    elements.push_back(e);
                }
            } else {
                elements.push_back(rhs);
            }

            left = new Tuple(left->token(), elements);
        } else {
            break;
        }
    }

    return left;
}

Parameter *Parser::read_parameter() {
    auto token = front_token();

    bool inout = is_and_skip_token(Token::InoutKeyword);

    auto name = read_name(false);
    return_if_null(name);

    return_if_false(skip_token(Token::AsKeyword));

    auto given_type = read_name(true);
    return_if_null(given_type);

    return new Parameter(token, inout, name, given_type);
}

Let *Parser::read_let() {
    auto lhs = read_variable_declaration();
    return_if_null(lhs);

    return_if_false(read_token(Token::Assignment, token));

    auto rhs = read_expression(true);
    return_if_null(rhs);

    ast::Expression *body = nullptr;
    if (is_and_skip_token(Token::Indent)) {
        body = read_expression();
        return_if_null(body);
        return_if_false(skip_deindent_and_end_token());
    }

    auto definition = new Let(lhs->token());
    definition->assignment = new Assignment(token, lhs, rhs);
    definition->set_body(body);
    return definition;
}

Def *Parser::read_def() {
    Token def_token;
    return_if_false(read_token(Token::DefKeyword, def_token));

    Name *name = nullptr;

    if (is_token(Token::Name)) {
        name = read_name(true);
    } else if (is_token(Token::Operator)) {
        name = read_operator(true);
    } else {
        report(SyntaxError(front_token(), "identifier or operator"));
        return nullptr;
    }

    return_if_null(name);

    std::vector<Parameter *> parameters;

    if (is_and_skip_token(Token::OpenParenthesis)) {
        while (!is_token(Token::CloseParenthesis)) {
            auto parameter = read_parameter();
            return_if_null(parameter);

            parameters.push_back(parameter);

            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_if_false(skip_token(Token::CloseParenthesis));
    }

    Name *given_return_type = nullptr;
    if (is_and_skip_token(Token::AsKeyword)) {
        given_return_type = read_name(true);
        return_if_null(given_return_type);
    }

    return_if_false(skip_token(Token::Indent));

    auto body = read_expression();
    return_if_null(body);

    return_if_false(skip_deindent_and_end_token());

    return new Def(def_token, name, parameters, body, given_return_type);
}

Type *Parser::read_type() {
    return_if_false(read_token(Token::TypeKeyword, token));

    auto definition = new Type(token);

    definition->set_name(read_name(true));

    if (is_and_skip_token(Token::AsKeyword)) {
        definition->alias = read_name(true);
    } else {
        return_if_false(skip_token(Token::Indent));

        while (!is_token(Token::Deindent)) {
            definition->field_names.push_back(read_name(false));
            skip_token(Token::AsKeyword);
            definition->field_types.push_back(read_name(true));
        }

        return_if_false(skip_deindent_and_end_token());
    }

    return definition;
}

Module *Parser::read_module() {
    return_if_false(read_token(Token::ModuleKeyword, token));

    auto name = read_name(false);
    return_if_null(name);

    auto body = read_block();
    return_if_null(name);

    return new Module(token, name, body);
}

Import *Parser::read_import_expression() {
    return_if_false(read_token(Token::ImportKeyword, token));
    auto path = read_string();
    return_if_null(path);
    return new Import(token, path);
}
