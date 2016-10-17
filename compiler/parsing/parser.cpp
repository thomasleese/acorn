//
// Created by Thomas Leese on 13/03/2016.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "../ast/nodes.h"
#include "../errors.h"
#include "lexer.h"

#include "parser.h"

using namespace acorn;
using namespace acorn::ast;

#define return_if_null(thing) if (thing == nullptr) return nullptr;
#define return_if_false(thing) if (thing == false) return nullptr;
#define return_eof_token(token) if (token.kind == Token::EndOfFile) return nullptr;

// useful variable for storing the current token
static Token token;

Parser::Parser(Lexer &lexer) : m_lexer(lexer) {
    m_operatorPrecendence["="] = 0;

    m_operatorPrecendence["+"] = 1;
    m_operatorPrecendence["-"] = 1;
}

Parser::~Parser() {

}

SourceFile *Parser::parse(std::string name) {
    SourceFile *module = new SourceFile(m_tokens.front(), name);

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
    while (!is_token(Token::EndOfFile)) {
        auto statement = readStatement();
        if (statement == nullptr) break;

        module->code->statements.push_back(statement);
    }

    return module;
}

void Parser::debug(std::string line) {
    //std::cerr << line << std::endl;
}

void Parser::next_token() {
    auto token = m_lexer.next_token();
    m_tokens.push_back(token);
}

bool Parser::read_token(Token::Kind kind, Token &token) {
    if (m_tokens.empty()) {
        next_token();
    }

    auto next_token = m_tokens.front();
    m_tokens.pop_front();

    if (next_token.kind != kind) {
        push_error(new errors::SyntaxError(token, kind));
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
    if (m_tokens.empty()) {
        next_token();
    }

    auto token = m_tokens.front();
    return token.kind == kind;
}

CodeBlock *Parser::readCodeBlock(bool in_switch) {
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
        read_token(Token::Deindent, token);
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
    auto token = read_token(Token::Operator);

    auto identifier = new Identifier(token, token.lexeme);

    if (accept_parameters && is_token(Token::OpenBrace)) {
        read_token(Token::OpenBrace);

        while (!is_token(Token::CloseBrace)) {
            identifier->parameters.push_back(readIdentifier(true));

            if (is_token(Token::Comma)) {
                read_token(Token::Comma);
            } else {
                break;
            }
        }

        read_token(Token::CloseBrace);
    }

    return identifier;
}

ast::VariableDeclaration *Parser::readVariableDeclaration() {
    auto token = read_token(Token::LetKeyword);
    return_eof_token(token);

    auto name = readIdentifier(false);
    return_if_null(name);

    ast::Identifier *type = nullptr;
    if (is_token(Token::AsKeyword)) {
        read_token(Token::AsKeyword);

        type = readIdentifier(true);
        return_if_null(type);
    }

    return new VariableDeclaration(token, name, type);
}

IntegerLiteral *Parser::readIntegerLiteral() {
    auto token = read_token(Token::IntegerLiteral);

    IntegerLiteral *literal = new IntegerLiteral(token);

    std::stringstream ss;
    ss << token.lexeme;
    ss >> literal->value;

    return literal;
}

FloatLiteral *Parser::readFloatLiteral() {
    auto token = read_token(Token::FloatLiteral);

    FloatLiteral *literal = new FloatLiteral(token);
    literal->value = token.lexeme;

    return literal;
}

StringLiteral *Parser::readStringLiteral() {
    auto token = read_token(Token::StringLiteral);

    StringLiteral *literal = new StringLiteral(token);
    literal->value = token.lexeme.substr(1, token.lexeme.length() - 2);

    return literal;
}

SequenceLiteral *Parser::readSequenceLiteral() {
    auto token = read_token(Token::OpenBracket);

    SequenceLiteral *literal = new SequenceLiteral(token);

    while (!is_token(Token::CloseBracket)) {
        literal->elements.push_back(readExpression(false));

        if (is_token(Token::Comma)) {
            read_token(Token::Comma);
        } else {
            break;
        }
    }

    read_token(Token::CloseBracket);

    return literal;
}

MappingLiteral *Parser::readMappingLiteral() {
    auto token = read_token(Token::OpenBrace);

    MappingLiteral *literal = new MappingLiteral(token);

    while (!is_token(Token::CloseBrace)) {
        Expression *key = readExpression(false);
        read_token(Token::Colon);
        Expression *value = readExpression(false);

        literal->keys.push_back(key);
        literal->values.push_back(value);

        if (is_token(Token::Comma)) {
            read_token(Token::Comma);
        } else {
            break;
        }
    }

    read_token(Token::CloseBrace);

    return literal;
}

RecordLiteral *Parser::readRecordLiteral() {
    auto token = read_token(Token::NewKeyword);

    auto literal = new RecordLiteral(token);

    literal->name = readIdentifier(true);

    read_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        literal->field_names.push_back(readIdentifier(false));
        read_token(Token::Colon);
        literal->field_values.push_back(readExpression(false));

        if (is_token(Token::Comma)) {
            read_token(Token::Comma);
        } else {
            break;
        }
    }

    read_token(Token::CloseParenthesis);

    return literal;
}

Call *Parser::readCall(Expression *operand) {
    auto token = read_token(Token::OpenParenthesis);

    Call *call = new Call(token);
    call->operand = operand;

    while (!is_token(Token::CloseParenthesis)) {
        call->arguments.push_back(readExpression(false));

        if (is_token(Token::Comma)) {
            read_token(Token::Comma);
        } else {
            break;
        }
    }

    read_token(Token::CloseParenthesis);

    return call;
}

CCall *Parser::readCCall() {
    auto token = read_token(Token::CCallKeyword);

    CCall *ccall = new CCall(token);
    ccall->name = readIdentifier(false);

    read_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        ccall->parameters.push_back(readIdentifier(true));

        if (is_token(Token::Comma)) {
            read_token(Token::Comma);
        } else {
            break;
        }
    }

    read_token(Token::CloseParenthesis);

    read_token(Token::AsKeyword);
    ccall->returnType = readIdentifier(true);

    if (is_token(Token::UsingKeyword)) {
        read_token(Token::UsingKeyword);

        while (true) {
            ccall->arguments.push_back(readExpression(false));

            if (is_token(Token::Comma)) {
                read_token(Token::Comma);
            } else {
                break;
            }
        }
    }

    return ccall;
}

Cast *Parser::readCast(Expression *operand) {
    auto token = read_token(Token::AsKeyword);

    Cast *cast = new Cast(token);
    cast->operand = operand;
    cast->new_type = readIdentifier(true);
    return cast;
}

Selector *Parser::readSelector(Expression *operand) {
    auto token = read_token(Token::Dot);
    return_eof_token(token);

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
    auto token = read_token(Token::OpenBracket);

    auto call = new Call(token);
    call->arguments.push_back(operand);
    call->arguments.push_back(readExpression(true));

    read_token(Token::CloseBracket);

    if (is_token(Token::Assignment)) {
        read_token(Token::Assignment);
        call->arguments.push_back(readExpression(true));
        call->operand = new Identifier(token, "setindex");
    } else {
        call->operand = new Identifier(token, "getindex");
    }

    return call;
}

While *Parser::readWhile() {
    auto token = read_token(Token::WhileKeyword);
    return_eof_token(token);

    auto condition = readExpression(true);
    return_if_null(condition);

    read_token(Token::Newline);

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

    auto token = read_token(Token::ForKeyword);
    return_eof_token(token);

    auto variable = readIdentifier(false);
    return_if_null(variable);

    return_eof_token(read_token(Token::InKeyword));

    auto iterator = readExpression(true);
    return_if_null(iterator);

    return_eof_token(read_token(Token::Newline));

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
    auto token = read_token(Token::IfKeyword);

    If *expression = new If(token);

    if (is_token(Token::LetKeyword)) {
        auto lhs = readVariableDeclaration();
        return_if_null(lhs);

        auto assignment_token = read_token(Token::Assignment);
        return_eof_token(assignment_token);

        auto rhs = readExpression(true);
        return_if_null(rhs);

        expression->condition = new Assignment(assignment_token, lhs, rhs);
    } else {
        expression->condition = readExpression(true);
        return_if_null(expression->condition);
    }

    return_eof_token(read_token(Token::Newline));

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
        read_token(Token::ElseKeyword);

        if (is_token(Token::IfKeyword)) {
            expression->falseCode = new CodeBlock(m_tokens.front());
            If *if_expr = readIf();
            expression->falseCode->statements.push_back(new ExpressionStatement(if_expr));
        } else {
            read_token(Token::Newline);
            expression->falseCode = readCodeBlock();
        }
    } else {
        read_token(Token::Deindent);
    }

    return expression;
}

Return *Parser::readReturn() {
    auto token = read_token(Token::ReturnKeyword);

    Return *r = new Return(token);
    r->expression = readExpression(true);
    return r;
}

Spawn *Parser::readSpawn() {
    auto token = read_token(Token::SpawnKeyword);

    Expression *expr = readExpression(true);
    Call *call = dynamic_cast<Call *>(expr);
    if (call) {
        return new Spawn(token, call);
    } else {
        push_error(new errors::SyntaxError(expr->token, "function call"));
        return nullptr;
    }
}

Case *Parser::readCase() {
    auto token = read_token(Token::CaseKeyword);
    return_eof_token(token);

    auto condition = readExpression(true);
    return_if_null(condition);

    Expression *assignment = nullptr;
    if (is_token(Token::UsingKeyword)) {
        read_token(Token::UsingKeyword);

        if (is_token(Token::LetKeyword)) {
            assignment = readVariableDeclaration();
        } else {
            assignment = readExpression(true);
        }
    }

    return_eof_token(read_token(Token::Newline));

    auto code = readCodeBlock(true);
    return_if_null(code);

    return new Case(token, condition, assignment, code);
}

Switch *Parser::readSwitch() {
    auto token = read_token(Token::SwitchKeyword);
    return_eof_token(token);

    auto expression = readExpression(true);
    return_if_null(expression);

    return_eof_token(read_token(Token::Newline));

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

    return_eof_token(read_token(Token::Deindent));

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
            read_token(Token::Assignment);
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
        read_token(Token::OpenParenthesis);
        Expression *expr = readExpression(true);
        read_token(Token::CloseParenthesis);
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
        if (is_token(Token::OpenParenthesis)) {
            left = readCall(left);
        } else if (is_token(Token::OpenBracket)) {
            left = readIndex(left);
        } else if (is_token(Token::AsKeyword)) {
            left = readCast(left);
        } else if (is_token(Token::Dot)) {
            left = readSelector(left);
        } else if (parse_comma && is_token(Token::Comma)) {
            read_token(Token::Comma);

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
        read_token(Token::InoutKeyword);
        parameter->inout = true;
    } else {
        parameter->inout = false;
    }

    parameter->name = readIdentifier(false);

    read_token(Token::AsKeyword);
    parameter->typeNode = readIdentifier(true);

    return parameter;
}

VariableDefinition *Parser::readVariableDefinition() {
    auto lhs = readVariableDeclaration();
    return_if_null(lhs);

    auto token = read_token(Token::Assignment);
    return_eof_token(token);

    auto rhs = readExpression(true);
    return_if_null(rhs);

    auto definition = new VariableDefinition(lhs->token);
    definition->name = lhs->name();
    definition->assignment = new Assignment(token, lhs, rhs);
    return definition;
}

FunctionDefinition *Parser::readFunctionDefinition() {
    debug("Reading FunctionDefinition...");

    auto token = read_token(Token::DefKeyword);

    FunctionDefinition *definition = new FunctionDefinition(token);

    if (is_token(Token::Name)) {
        definition->name = readIdentifier(true);
    } else if (is_token(Token::Operator)) {
        definition->name = readOperator(true);
    } else {
        push_error(new errors::SyntaxError(m_tokens.front(), "identifier or operator"));
        return nullptr;
    }

    debug("Name: " + definition->name->value);

    read_token(Token::OpenParenthesis);

    while (!is_token(Token::CloseParenthesis)) {
        definition->parameters.push_back(readParameter());

        if (is_token(Token::Comma)) {
            read_token(Token::Comma);
        } else {
            break;  // no more parameters, apparently
        }
    }

    read_token(Token::CloseParenthesis);

    read_token(Token::AsKeyword);
    definition->returnType = readIdentifier(true);

    read_token(Token::Newline);

    definition->code = readCodeBlock();

    debug("Ending FunctionDefinition!");

    return definition;
}

TypeDefinition *Parser::readTypeDefinition() {
    auto token = read_token(Token::TypeKeyword);

    TypeDefinition *definition = new TypeDefinition(token);

    definition->name = readIdentifier(true);

    if (is_token(Token::AsKeyword)) {
        read_token(Token::AsKeyword);
        definition->alias = readIdentifier(true);
    } else {
        read_token(Token::Newline);

        while (!is_token(Token::Deindent)) {
            definition->field_names.push_back(readIdentifier(false));
            read_token(Token::AsKeyword);
            definition->field_types.push_back(readIdentifier(true));
            read_token(Token::Newline);
        }

        read_token(Token::Deindent);
    }

    return definition;
}

ast::ProtocolDefinition *Parser::readProtocolDefinition() {
    auto token = read_token(Token::ProtocolKeyword);

    auto name = readIdentifier(true);
    std::vector<MethodSignature *> methods;

    read_token(Token::Newline);

    while (!is_token(Token::Deindent)) {
        auto method_token = read_token(Token::DefKeyword);
        return_eof_token(method_token);

        auto method_name = readIdentifier(false);
        return_if_null(method_name);

        std::vector<Identifier *> parameter_types;

        return_eof_token(read_token(Token::OpenParenthesis));
        while (!is_token(Token::CloseParenthesis)) {
            parameter_types.push_back(readIdentifier(true));

            if (is_token(Token::Comma)) {
                read_token(Token::Comma);
            } else {
                break;  // no more parameters, apparently
            }
        }

        read_token(Token::CloseParenthesis);

        read_token(Token::AsKeyword);
        auto return_type = readIdentifier(true);
        return_if_null(return_type);

        return_eof_token(read_token(Token::Newline));

        methods.push_back(new MethodSignature(method_token, method_name, parameter_types, return_type));
    }

    read_token(Token::Deindent);

    return new ProtocolDefinition(token, name, methods);
}

ast::EnumDefinition *Parser::readEnumDefinition() {
    auto token = read_token(Token::EnumKeyword);
    return_eof_token(token);

    auto name = readIdentifier(true);
    return_if_null(name);

    std::vector<EnumElement *> elements;

    read_token(Token::Newline);

    while (!is_token(Token::Deindent)) {
        auto element_name = readIdentifier(false);
        return_if_null(element_name);

        ast::Identifier *element_type = nullptr;

        if (is_token(Token::AsKeyword)) {
            read_token(Token::AsKeyword);

            element_type = readIdentifier(true);
            return_if_null(element_type);
        }

        return_eof_token(read_token(Token::Newline));

        elements.push_back(new EnumElement(element_name->token, element_name, element_type));
    }

    read_token(Token::Deindent);

    return new EnumDefinition(token, name, elements);
}

Statement *Parser::readStatement() {
    debug("Reading Statement...");

    Statement *statement;

    if (is_token(Token::LetKeyword)) {
        auto definition = readVariableDefinition();
        return_if_null(definition);
        statement = new DefinitionStatement(definition);
    } else if (is_token(Token::DefKeyword)) {
        statement = new DefinitionStatement(readFunctionDefinition());
    } else if (is_token(Token::TypeKeyword)) {
        statement = new DefinitionStatement(readTypeDefinition());
    } else if (is_token(Token::ProtocolKeyword)) {
        statement = new DefinitionStatement(readProtocolDefinition());
    } else if (is_token(Token::EnumKeyword)) {
        statement = new DefinitionStatement(readEnumDefinition());
    } else {
        auto expression = readExpression(true);
        return_if_null(expression);

        statement = new ExpressionStatement(expression);
    }

    return_eof_token(read_token(Token::Newline));

    return statement;
}

ImportStatement *Parser::readImportStatement() {
    auto token = read_token(Token::ImportKeyword);
    StringLiteral *path = readStringLiteral();
    read_token(Token::Newline);

    return new ImportStatement(token, path);
}
