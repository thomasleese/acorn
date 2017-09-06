//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <memory>
#include <sstream>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/diagnostics.h"
#include "acorn/parser/lexer.h"
#include "acorn/utils.h"

#include "acorn/parser/parser.h"

using namespace acorn;
using namespace acorn::ast;
using namespace acorn::diagnostics;
using namespace acorn::parser;

static auto logger = spdlog::get("acorn");

// useful variable for storing the current token
static Token token;

Parser::Parser(Lexer &lexer) : m_lexer(lexer) {
    logger->debug("Initialising parser...");

    m_operator_precendence["+"] = 1;
    m_operator_precendence["-"] = 1;
}

std::unique_ptr<SourceFile> Parser::parse(std::string name) {
    logger->info("Parsing: {}", name);

    return_null_if_false(fill_token());
    Token source_token = front_token();

    std::vector<std::unique_ptr<SourceFile>> imports;

    while (is_keyword("import")) {
        auto import = read_import_expression();
        return_null_if_null(import);

        std::string filename = "../library/" + import->path()->value() + ".acorn";

        Lexer lexer(filename);
        Parser parser(lexer);

        auto imported_module = parser.parse(filename);

        if (lexer.has_errors() || parser.has_errors()) {
            continue;
        }

        imports.push_back(std::move(imported_module));
    }

    Token block_token = front_token();

    std::vector<std::unique_ptr<Node>> expressions;
    while (!is_token(Token::EndOfFile)) {
        auto expression = read_expression();
        return_null_if_null(expression);
        expressions.push_back(std::move(expression));
    }

    auto code = std::make_unique<Block>(block_token, std::move(expressions));

    return std::make_unique<SourceFile>(
        source_token, name, std::move(imports), std::move(code)
    );
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
    return skip_token(Token::Deindent) && skip_keyword("end");
}

bool Parser::read_keyword(const std::string &name, Token &token) {
    Token next_front_token;
    if (!read_token(Token::Keyword, next_front_token)) {
        return false;
    }

    if (next_front_token.lexeme != name) {
        report(SyntaxError(next_front_token, name));
        return false;
    }

    token = next_front_token;

    return true;
}

bool Parser::skip_keyword(const std::string &name) {
    Token tmp_token;
    return read_keyword(name, tmp_token);
}

bool Parser::is_keyword(const std::string &name) {
    if (!is_token(Token::Keyword)) {
        return false;
    }

    return front_token().lexeme == name;
}

bool Parser::is_and_skip_keyword(const std::string &name) {
    return is_keyword(name) && skip_keyword(name);
}

std::unique_ptr<Block> Parser::read_block(bool read_end) {
    Token block_token;
    return_null_if_false(read_token(Token::Indent, block_token));

    std::vector<std::unique_ptr<Node>> expressions;

    while (!is_token(Token::Deindent)) {
        auto expression = read_expression();
        return_null_if_null(expression);
        expressions.push_back(std::move(expression));
    }

    return_null_if_false(skip_token(Token::Deindent));

    if (read_end) {
        return_null_if_false(skip_keyword("End"));
    }

    return std::make_unique<Block>(block_token, std::move(expressions));
}

std::unique_ptr<Node> Parser::read_expression(bool parse_comma) {
    if (is_keyword("let")) {
        return read_let();
    } else if (is_keyword("def")) {
        return read_def();
    } else if (is_keyword("type")) {
        return read_type_decl();
    } else if (is_keyword("module")) {
        return read_module();
    } else {
        auto unary_expression = read_unary_expression(parse_comma);
        return_null_if_null(unary_expression);

        if (is_token(Token::Operator) || is_token(Token::Assignment)) {
            return read_binary_expression(std::move(unary_expression), 0);
        } else {
            return unary_expression;
        }
    }
}

std::unique_ptr<Name> Parser::read_name() {
    Token name_token;
    return_null_if_false(read_token(Token::Name, name_token));

    return std::make_unique<Name>(name_token, name_token.lexeme);
}

std::unique_ptr<Name> Parser::read_operator() {
    Token name_token;
    return_null_if_false(read_token(Token::Operator, name_token));

    return std::make_unique<Name>(name_token, name_token.lexeme);
}

std::unique_ptr<TypeName> Parser::read_type_name() {
    auto name = read_name();

    std::vector<std::unique_ptr<TypeName>> parameters;

    if (is_and_skip_token(Token::OpenBrace)) {
        while (!is_token(Token::CloseBrace)) {
            auto parameter = read_type_name();
            return_null_if_null(parameter);
            parameters.push_back(std::move(parameter));

            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_null_if_false(skip_token(Token::CloseBrace));
    }

    return std::make_unique<TypeName>(
        name->token(), std::move(name), std::move(parameters)
    );
}

std::unique_ptr<ast::DeclName> Parser::read_decl_name(bool can_be_operator) {
    std::unique_ptr<Name> name;

    if (can_be_operator && is_token(Token::Operator)) {
        name = read_operator();
    } else {
        name = read_name();
    }

    std::vector<std::unique_ptr<Name>> parameters;

    if (is_and_skip_token(Token::OpenBrace)) {
        while (!is_token(Token::CloseBrace)) {
            auto parameter = read_name();
            return_null_if_null(parameter);
            parameters.push_back(std::move(parameter));

            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_null_if_false(skip_token(Token::CloseBrace));
    }

    return std::make_unique<DeclName>(
        name->token(), std::move(name), std::move(parameters)
    );
}

std::unique_ptr<ParamName> Parser::read_param_name() {
    auto name = read_name();

    std::vector<std::unique_ptr<TypeName>> parameters;

    if (is_and_skip_token(Token::OpenBrace)) {
        while (!is_token(Token::CloseBrace)) {
            auto parameter = read_type_name();
            return_null_if_null(parameter);
            parameters.push_back(std::move(parameter));

            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_null_if_false(skip_token(Token::CloseBrace));
    }

    return std::make_unique<ParamName>(
        name->token(), std::move(name), std::move(parameters)
    );
}

std::unique_ptr<ParamName> Parser::read_param_operator() {
    auto name = read_operator();

    std::vector<std::unique_ptr<TypeName>> parameters;

    if (is_and_skip_token(Token::OpenBrace)) {
        while (!is_token(Token::CloseBrace)) {
            auto parameter = read_type_name();
            return_null_if_null(parameter);
            parameters.push_back(std::move(parameter));

            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_null_if_false(skip_token(Token::CloseBrace));
    }

    return std::make_unique<ParamName>(
        name->token(), std::move(name), std::move(parameters)
    );
}

std::unique_ptr<VarDecl> Parser::read_var_decl() {
    Token let_token;
    return_null_if_false(read_keyword("let", let_token));

    bool builtin = is_and_skip_keyword("builtin");

    auto name = read_decl_name();
    return_null_if_null(name);

    std::unique_ptr<TypeName> type_name;
    if (is_and_skip_keyword("as")) {
        type_name = read_type_name();
        return_null_if_null(type_name);
    }

    return std::make_unique<VarDecl>(
        let_token, std::move(name), std::move(type_name), builtin
    );
}

std::unique_ptr<Int> Parser::read_int() {
    return_null_if_false(read_token(Token::Int, token));
    return std::make_unique<Int>(token, token.lexeme);
}

std::unique_ptr<Float> Parser::read_float() {
    return_null_if_false(read_token(Token::Float, token));
    return std::make_unique<Float>(token, token.lexeme);
}

std::unique_ptr<String> Parser::read_string() {
    return_null_if_false(read_token(Token::String, token));
    return std::make_unique<String>(token, token.lexeme);
}

std::unique_ptr<List> Parser::read_list() {
    Token list_token;
    return_null_if_false(read_token(Token::OpenBracket, list_token));

    std::vector<std::unique_ptr<Node>> elements;

    while (!is_token(Token::CloseBracket)) {
        auto element = read_expression(false);
        return_null_if_null(element);

        elements.push_back(std::move(element));

        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_null_if_false(skip_token(Token::CloseBracket));

    return std::make_unique<List>(list_token, std::move(elements));
}

std::unique_ptr<Dictionary> Parser::read_dictionary() {
    Token dict_token;
    return_null_if_false(read_token(Token::OpenBrace, dict_token));

    std::vector<std::unique_ptr<Node>> keys;
    std::vector<std::unique_ptr<Node>> values;

    while (!is_token(Token::CloseBrace)) {
        auto key = read_expression(false);
        return_null_if_null(key);
        keys.push_back(std::move(key));

        return_null_if_false(skip_token(Token::Colon));

        auto value = read_expression(false);
        return_null_if_null(value);
        values.push_back(std::move(value));

        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_null_if_false(skip_token(Token::CloseBrace));

    return std::make_unique<Dictionary>(
        dict_token, std::move(keys), std::move(values)
    );
}

std::unique_ptr<Call> Parser::read_call(std::unique_ptr<Node> operand) {
    Token call_token;
    return_null_if_false(read_token(Token::OpenParenthesis, call_token));

    std::vector<std::unique_ptr<Node>> positional_arguments;
    std::map<std::string, std::unique_ptr<Node>> keyword_arguments;

    while (!is_token(Token::CloseParenthesis)) {
        auto saved_token = front_token();

        std::unique_ptr<Name> name = nullptr;
        auto expression = read_expression();

        if (is_and_skip_token(Token::Colon)) {
            if (!llvm::isa<Name>(expression.get())) {
                report(SyntaxError(saved_token, Token::Name));
                return nullptr;
            }

            name = std::unique_ptr<Name>(static_cast<Name *>(expression.release()));
            expression = read_expression(false);
        }

        return_null_if_null(expression);

        if (name == nullptr) {
            positional_arguments.push_back(std::move(expression));
        } else {
            keyword_arguments[name->value()] = std::move(expression);
        }

        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_null_if_false(skip_token(Token::CloseParenthesis));

    return std::make_unique<Call>(
        call_token, std::move(operand), std::move(positional_arguments), std::move(keyword_arguments)
    );
}

std::unique_ptr<CCall> Parser::read_ccall() {
    Token ccall_token;
    return_null_if_false(read_keyword("ccall", ccall_token));

    auto name = read_name();
    return_null_if_null(name);

    return_null_if_false(skip_token(Token::OpenParenthesis));

    std::vector<std::unique_ptr<TypeName>> parameters;
    while (!is_token(Token::CloseParenthesis)) {
        auto parameter = read_type_name();
        return_null_if_null(parameter);
        parameters.push_back(std::move(parameter));

        if (!is_and_skip_token(Token::Comma)) {
            break;
        }
    }

    return_null_if_false(skip_token(Token::CloseParenthesis));
    return_null_if_false(skip_keyword("as"));

    auto return_type = read_type_name();
    return_null_if_null(return_type);

    std::vector<std::unique_ptr<Node>> arguments;
    if (is_and_skip_keyword("using")) {
        while (true) {
            auto argument = read_expression(false);
            return_null_if_null(argument);
            arguments.push_back(std::move(argument));
            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }
    }

    return std::make_unique<CCall>(
        ccall_token, std::move(name), std::move(parameters),
        std::move(return_type), std::move(arguments)
    );
}

std::unique_ptr<Cast> Parser::read_cast(std::unique_ptr<Node> operand) {
    Token as_token;
    return_null_if_false(read_keyword("as", as_token));

    auto new_type = read_type_name();
    return_null_if_null(new_type);

    return std::make_unique<Cast>(
        as_token, std::move(operand), std::move(new_type)
    );
}

std::unique_ptr<Selector> Parser::read_selector(std::unique_ptr<Node> operand, bool allow_operators) {
    return_null_if_false(read_token(Token::Dot, token));

    std::unique_ptr<ParamName> name;

    if (is_token(Token::Int)) {
        auto il = read_int();
        name = std::make_unique<ParamName>(il->token(), il->value());
    } else {
        if (allow_operators && is_token(Token::Operator)) {
            name = read_param_operator();
        } else {
            name = read_param_name();
        }
    }

    return_null_if_null(name);

    return std::make_unique<Selector>(token, std::move(operand), std::move(name));
}

std::unique_ptr<Call> Parser::read_index(std::unique_ptr<Node> operand) {
    Token call_token;
    return_null_if_false(read_token(Token::OpenBracket, call_token));

    auto index = read_expression(true);
    return_null_if_null(index);

    return_null_if_false(skip_token(Token::CloseBracket));

    std::vector<std::unique_ptr<Node>> arguments;
    arguments.push_back(std::move(operand));
    arguments.push_back(std::move(index));

    if (is_and_skip_token(Token::Assignment)) {
        auto value = read_expression(true);
        return_null_if_null(value);

        arguments.push_back(std::move(value));

        return std::make_unique<Call>(call_token, "setindex", std::move(arguments));
    } else {
        return std::make_unique<Call>(call_token, "getindex", std::move(arguments));
    }
}

std::unique_ptr<While> Parser::read_while() {
    Token while_token;
    return_null_if_false(read_keyword("while", while_token));

    auto condition = read_expression(true);
    return_null_if_null(condition);

    return_null_if_false(skip_token(Token::Newline));
    return_null_if_false(skip_token(Token::Indent));

    auto body = read_expression();
    return_null_if_null(body);

    return_null_if_false(skip_token(Token::Deindent));
    return_null_if_false(skip_keyword("end"));

    return std::make_unique<While>(
        while_token, std::move(condition), std::move(body)
    );
}

std::unique_ptr<Block> Parser::read_for() {
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

    return_null_if_false(read_keyword("for", token));

    auto variable = read_name();
    return_null_if_null(variable);

    return_null_if_false(skip_keyword("in"));

    auto iterator = read_expression(true);
    return_null_if_null(iterator);

    return_null_if_false(skip_token(Token::Newline));

    auto loop_code = read_block();
    return_null_if_null(loop_code);

    std::string for_id;
    std::stringstream ss;
    ss << &token;
    ss >> for_id;

    std::string state_variable_name = "state_" + for_id;
    std::string next_state_variable_name = "next_state_" + for_id;

    /*auto code_block = new Block(token);

    auto state_variable = std::make_unique<Let>(token, state_variable_name, new Call(token, "start", std::move(iterator)));
    code_block->add_expression(std::move(state_variable));

    auto condition = new Call(token, "not", new Call(token, "done", iterator, static_cast<VarDecl *>(&state_variable->assignment->lhs())->name()));
    auto while_code = new While(token, condition, std::move(loop_code));

    auto next_state_variable = new Let(token, next_state_variable_name, new Call(token, "next", iterator, static_cast<VarDecl *>(&state_variable->assignment->lhs())->name()));
    loop_code->insert_expression(0, next_state_variable);
    loop_code->insert_expression(1, new Assignment(token, static_cast<VarDecl *>(&state_variable->assignment->lhs()), new Selector(token, static_cast<VarDecl *>(&next_state_variable->assignment->lhs())->name(), "1")));
    loop_code->insert_expression(1, new Assignment(token, new VarDecl(token, new Name(token, next_state_variable_name), nullptr, false), new Selector(token, static_cast<VarDecl *>(&next_state_variable->assignment->lhs())->name(), "0")));

    code_block->add_expression(while_code);

    return code_block;*/

    std::vector<std::unique_ptr<Node>> expressions;
    return std::make_unique<Block>(token, std::move(expressions));
}

std::unique_ptr<If> Parser::read_if() {
    Token if_token;
    return_null_if_false(read_keyword("if", if_token));

    std::unique_ptr<Node> condition;

    if (is_keyword("let")) {
        auto lhs = read_var_decl();
        return_null_if_null(lhs);

        Token assignment_token;
        return_null_if_false(read_token(Token::Assignment, assignment_token));

        auto rhs = read_expression(true);
        return_null_if_null(rhs);

        condition = std::make_unique<Assignment>(
            assignment_token, std::move(lhs), std::move(rhs)
        );
    } else {
        condition = read_expression();
    }

    return_null_if_null(condition);

    return_null_if_false(skip_token(Token::Indent));

    auto true_case = read_expression();
    std::unique_ptr<Node> false_case;

    return_null_if_false(skip_token(Token::Deindent));

    if (is_and_skip_keyword("else")) {
        if (is_keyword("if")) {
            false_case = read_if();
        } else {
            return_null_if_false(skip_token(Token::Indent));
            false_case = read_expression();
            return_null_if_false(skip_deindent_and_end_token());
        }
    } else {
        return_null_if_false(skip_deindent_and_end_token());
    }

    return std::make_unique<If>(
        if_token, std::move(condition), std::move(true_case), std::move(false_case)
    );
}

std::unique_ptr<Return> Parser::read_return() {
    Token return_token;
    return_null_if_false(read_keyword("return", return_token));

    auto expression = read_expression();
    return_null_if_null(expression);

    return std::make_unique<Return>(return_token, std::move(expression));
}

std::unique_ptr<Spawn> Parser::read_spawn() {
    Token spawn_token;
    return_null_if_false(read_keyword("spawn", spawn_token));

    auto expression = read_expression();
    return_null_if_null(expression);

    if (llvm::isa<Call>(expression.get())) {
        auto call = std::unique_ptr<Call>(static_cast<Call *>(expression.release()));
        return std::make_unique<Spawn>(spawn_token, std::move(call));
    } else {
        report(SyntaxError(expression->token(), "function call"));
        return nullptr;
    }
}

std::unique_ptr<Case> Parser::read_case() {
    Token case_token;
    return_null_if_false(read_keyword("case", case_token));

    auto condition = read_expression();
    return_null_if_null(condition);

    std::unique_ptr<Node> assignment;

    if (is_and_skip_keyword("using")) {
        if (is_keyword("let")) {
            assignment = std::unique_ptr<VarDecl>(read_var_decl());
        } else {
            assignment = read_expression(true);
        }
    }

    return_null_if_null(assignment);
    return_null_if_false(skip_token(Token::Newline));

    auto code = read_block(false);
    return_null_if_null(code);

    return std::make_unique<Case>(
        token, std::move(condition), std::move(assignment), std::move(code)
    );
}

std::unique_ptr<Switch> Parser::read_switch() {
    Token switch_token;
    return_null_if_false(read_keyword("switch", switch_token));

    auto expression = read_expression(true);
    return_null_if_null(expression);

    return_null_if_false(skip_token(Token::Newline));

    std::vector<std::unique_ptr<Case>> cases;
    while (is_keyword("case")) {
        auto entry = read_case();
        return_null_if_null(entry);
        cases.push_back(std::move(entry));
    }

    std::unique_ptr<Block> default_block;
    if (is_keyword("default")) {
        default_block = read_block(false);
        return_null_if_null(default_block);
    }

    return_null_if_false(skip_keyword("end"));

    return std::make_unique<Switch>(
        token, std::move(expression), std::move(cases), std::move(default_block)
    );
}

std::unique_ptr<Node> Parser::read_unary_expression(bool parse_comma) {
    if (is_token(Token::Operator)) {
        auto operand = read_param_operator();
        return_null_if_null(operand);

        auto argument = read_unary_expression(false);
        return_null_if_null(argument);

        return std::make_unique<Call>(
            operand->token(), std::move(operand), std::move(argument)
        );
    } else {
        return read_operand_expression(parse_comma);
    }
}

std::unique_ptr<Node> Parser::read_binary_expression(std::unique_ptr<Node> lhs, int min_precedence) {
    while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operator_precendence[front_token().lexeme] >= min_precedence) {
        auto saved_token = front_token();

        auto op = read_param_operator();
        return_null_if_null(op);

        auto rhs = read_operand_expression(true);
        return_null_if_null(rhs);

        while ((is_token(Token::Operator) || is_token(Token::Assignment)) && m_operator_precendence[front_token().lexeme] > m_operator_precendence[op->value()]) {
            rhs = read_binary_expression(std::move(rhs), m_operator_precendence[front_token().lexeme]);
        }

        lhs = std::make_unique<Call>(saved_token, std::move(op), std::move(lhs), std::move(rhs));
    }

    return lhs;
}

std::unique_ptr<Node> Parser::read_parenthesis_expression() {
    return_null_if_false(skip_token(Token::OpenParenthesis));
    auto expression = read_expression(true);
    return_null_if_null(expression);
    return_null_if_false(skip_token(Token::CloseParenthesis));
    return expression;
}

std::unique_ptr<Node> Parser::read_primary_expression() {
    if (is_token(Token::OpenParenthesis)) {
        return std::unique_ptr<Node>(read_parenthesis_expression());
    } else if (is_token(Token::Int)) {
        return std::unique_ptr<Node>(read_int());
    } else if (is_token(Token::Float)) {
        return std::unique_ptr<Node>(read_float());
    } else if (is_token(Token::String)) {
        return std::unique_ptr<Node>(read_string());
    } else if (is_token(Token::OpenBracket)) {
        return std::unique_ptr<Node>(read_list());
    } else if (is_token(Token::OpenBrace)) {
        return std::unique_ptr<Node>(read_dictionary());
    } else if (is_keyword("while")) {
        return std::unique_ptr<Node>(read_while());
    } else if (is_keyword("for")) {
        return std::unique_ptr<Node>(read_for());
    } else if (is_keyword("if")) {
        return read_if();
    } else if (is_keyword("switch")) {
        return std::unique_ptr<Node>(read_switch());
    } else if (is_keyword("return")) {
        return std::unique_ptr<Node>(read_return());
    } else if (is_keyword("spawn")) {
        return std::unique_ptr<Node>(read_spawn());
    } else if (is_keyword("ccall")) {
        return std::unique_ptr<Node>(read_ccall());
    } else if (is_token(Token::Name)) {
        return std::unique_ptr<Node>(read_param_name());
    } else {
        report(SyntaxError(front_token(), "primary expression"));
        return nullptr;
    }
}

std::unique_ptr<Node> Parser::read_operand_expression(bool parse_comma) {
    auto left = read_primary_expression();
    return_null_if_null(left);

    while (true) {
        if (is_token(Token::OpenParenthesis)) {
            left = read_call(std::move(left));
        } else if (is_token(Token::OpenBracket)) {
            left = read_index(std::move(left));
        } else if (is_keyword("as")) {
            left = read_cast(std::move(left));
        } else if (is_token(Token::Dot)) {
            left = read_selector(std::move(left));
        } else {
            break;
        }
    }

    return left;
}

std::unique_ptr<Parameter> Parser::read_parameter() {
    auto token = front_token();

    bool inout = is_and_skip_keyword("inout");

    auto name = read_name();
    return_null_if_null(name);

    std::unique_ptr<TypeName> given_type;

    if (is_and_skip_keyword("as")) {
        given_type = read_type_name();
        return_null_if_null(given_type);
    }

    return std::make_unique<Parameter>(
        token, inout, std::move(name), std::move(given_type)
    );
}

std::unique_ptr<Let> Parser::read_let() {
    auto lhs = read_var_decl();
    return_null_if_null(lhs);

    std::unique_ptr<Node> rhs;

    if (!lhs->builtin()) {
        return_null_if_false(read_token(Token::Assignment, token));

        rhs = read_expression(true);
        return_null_if_null(rhs);
    }

    std::unique_ptr<Node> body;

    if (is_and_skip_token(Token::Indent)) {
        body = read_expression();
        return_null_if_null(body);
        return_null_if_false(skip_deindent_and_end_token());
    }

    auto assignment = std::make_unique<Assignment>(
        token, std::move(lhs), std::move(rhs)
    );

    return std::make_unique<Let>(
        assignment->lhs()->token(), std::move(assignment), std::move(body)
    );
}

std::unique_ptr<DefInstance> Parser::read_def_instance() {
    Token def_token;
    return_null_if_false(read_keyword("def", def_token));

    bool builtin = is_and_skip_keyword("builtin");

    auto name = read_decl_name(true);
    return_null_if_null(name);

    std::vector<std::unique_ptr<Parameter>> parameters;

    if (is_and_skip_token(Token::OpenParenthesis)) {
        while (!is_token(Token::CloseParenthesis)) {
            auto parameter = read_parameter();
            return_null_if_null(parameter);

            parameters.push_back(std::move(parameter));

            if (!is_and_skip_token(Token::Comma)) {
                break;
            }
        }

        return_null_if_false(skip_token(Token::CloseParenthesis));
    }

    std::unique_ptr<TypeName> return_type;

    if (builtin) {
        return_null_if_false(skip_keyword("as"));
        return_type = read_type_name();
        return_null_if_null(return_type);
    } else if (is_and_skip_keyword("as")) {
        return_type = read_type_name();
        return_null_if_null(return_type);
    }

    std::unique_ptr<Node> body;

    if (!builtin) {
        return_null_if_false(skip_token(Token::Indent));
        body = read_expression();
        return_null_if_null(body);
        return_null_if_false(skip_deindent_and_end_token());
    }

    return std::make_unique<DefInstance>(
        def_token, std::move(name), builtin, std::move(parameters),
        std::move(body), std::move(return_type)
    );
}

std::unique_ptr<Def> Parser::read_def() {
    auto def_token = front_token();

    auto instance = read_def_instance();
    return_null_if_null(instance);

    return std::make_unique<Def>(def_token, std::move(instance));
}

std::unique_ptr<TypeDecl> Parser::read_type_decl() {
    Token type_token;
    return_null_if_false(read_keyword("type", type_token));

    bool builtin = is_and_skip_keyword("builtin");

    auto name = read_decl_name();
    return_null_if_null(name);

    if (builtin) {
        return std::make_unique<TypeDecl>(type_token, std::move(name));
    }

    if (is_and_skip_keyword("as")) {
        auto alias = read_type_name();
        return_null_if_null(alias);

        return std::make_unique<TypeDecl>(
            type_token, std::move(name), std::move(alias)
        );
    } else {
        return_null_if_false(skip_token(Token::Indent));

        std::vector<std::unique_ptr<Name>> field_names;
        std::vector<std::unique_ptr<TypeName>> field_types;

        while (!is_token(Token::Deindent)) {
            auto field_name = read_name();
            return_null_if_null(field_name);
            field_names.push_back(std::move(field_name));

            return_null_if_false(skip_keyword("as"));

            auto field_type = read_type_name();
            return_null_if_null(field_type);
            field_types.push_back(std::move(field_type));
        }

        return_null_if_false(skip_deindent_and_end_token());

        return std::make_unique<TypeDecl>(
            type_token, std::move(name),
            std::move(field_names), std::move(field_types)
        );
    }
}

std::unique_ptr<Module> Parser::read_module() {
    Token module_token;
    return_null_if_false(read_keyword("module", module_token));

    auto name = read_decl_name();
    return_null_if_null(name);

    auto body = read_block();
    return_null_if_null(name);

    return std::make_unique<Module>(
        module_token, std::move(name), std::move(body)
    );
}

std::unique_ptr<Import> Parser::read_import_expression() {
    Token import_token;
    return_null_if_false(read_keyword("import", import_token));

    auto path = read_string();
    return_null_if_null(path);

    return std::make_unique<Import>(import_token, std::move(path));
}
