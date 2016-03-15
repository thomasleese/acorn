//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>

#include "Errors.h"

using namespace Errors;

std::string rule_string(Lexer::Rule rule) {
    switch (rule) {
        case Lexer::Whitespace:
            return "whitespace";
        case Lexer::Newline:
            return "new line";
            break;
        case Lexer::EndOfFile:
            return "end of file";
            break;
        case Lexer::LetKeyword:
            return "let";
            break;
        case Lexer::DefKeyword:
            return "def";
            break;
        case Lexer::TypeKeyword:
            return "type";
            break;
        case Lexer::AsKeyword:
            return "as";
            break;
        case Lexer::EndKeyword:
            return "end";
            break;
        case Lexer::WhileKeyword:
            return "while";
            break;
        case Lexer::ForKeyword:
            return "for";
            break;
        case Lexer::InKeyword:
            return "in";
            break;
        case Lexer::IfKeyword:
            return "if";
            break;
        case Lexer::ElseKeyword:
            return "else";
            break;
        case Lexer::BooleanLiteral:
            return "boolean";
            break;
        case Lexer::IntegerLiteral:
            return "integer";
            break;
        case Lexer::FloatLiteral:
            return "float";
            break;
        case Lexer::StringLiteral:
            return "string";
            break;
        case Lexer::ComplexLiteral:
            return "complex";
            break;
        case Lexer::Identifier:
            return "name";
            break;
        case Lexer::Operator:
            return "operator";
            break;
        case Lexer::OpenBracket:
            return "[";
            break;
        case Lexer::CloseBracket:
            return "]";
            break;
        case Lexer::OpenParenthesis:
            return "(";
            break;
        case Lexer::CloseParenthesis:
            return ")";
            break;
        case Lexer::OpenBrace:
            return "{";
            break;
        case Lexer::CloseBrace:
            return "}";
            break;
        case Lexer::OpenChevron:
            return "<";
            break;
        case Lexer::CloseChevron:
            return ">";
            break;
        case Lexer::Comma:
            return ",";
            break;
        case Lexer::Dot:
            return ".";
        case Lexer::Colon:
            return ":";
    }
}

CompilerError::CompilerError(std::string filename) {
    m_filename = filename;
}

const char* CompilerError::what() const noexcept {
    return "CompilerError";
}

SyntaxError::SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation) : CompilerError(filename) {
    m_lineNumber = lineNumber;
    m_column = column;
    m_line = line;
    m_got = got;
    m_expectation = expectation;
}

SyntaxError::SyntaxError(Lexer::Token *token, std::string expectation) : CompilerError(token->filename) {
    m_lineNumber = token->lineNumber;
    m_column = token->column;
    m_line = token->line;
    m_got = token->lexeme;

    if (m_got.empty()) {
        m_got = "(" + rule_string(token->rule) + ")";
    }

    m_expectation = expectation;
}

SyntaxError::SyntaxError(Lexer::Token *token, Lexer::Rule rule) : CompilerError(token->filename) {
    m_lineNumber = token->lineNumber;
    m_column = token->column;
    m_line = token->line;
    m_got = token->lexeme;

    if (m_got.empty()) {
        m_got = "(" + rule_string(token->rule) + ")";
    }

    m_expectation = rule_string(rule);
}

void SyntaxError::print() const {
    std::cout << "Invalid syntax in " << m_filename << " on line " << m_lineNumber << " column " << m_column << std::endl;
    std::cout << std::endl;

    std::cout << "    " << m_line << std::endl;

    std::cout << "    ";
    for (int i = 0; i < m_column; i++) {
        std::cout << " ";
    }
    std::cout << "^" << std::endl;
    std::cout << std::endl;

    std::cout << "Expected: " << m_expectation << std::endl;
    std::cout << "Got: " << m_got << std::endl;
}
