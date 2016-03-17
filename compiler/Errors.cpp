//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "AbstractSyntaxTree.h"
#include "Errors.h"

using namespace Errors;

std::string Errors::rule_string(Lexer::Rule rule) {
    switch (rule) {
        case Lexer::Whitespace:
            return "whitespace";
        case Lexer::Newline:
            return "new line";
        case Lexer::Comment:
            return "comment";
        case Lexer::EndOfFile:
            return "end of file";
        case Lexer::LetKeyword:
            return "let";
        case Lexer::DefKeyword:
            return "def";
        case Lexer::TypeKeyword:
            return "type";
        case Lexer::AsKeyword:
            return "as";
        case Lexer::EndKeyword:
            return "end";
        case Lexer::WhileKeyword:
            return "while";
        case Lexer::ForKeyword:
            return "for";
        case Lexer::InKeyword:
            return "in";
        case Lexer::IfKeyword:
            return "if";
        case Lexer::ElseKeyword:
            return "else";
        case Lexer::BooleanLiteral:
            return "boolean";
        case Lexer::IntegerLiteral:
            return "integer";
        case Lexer::FloatLiteral:
            return "float";
        case Lexer::StringLiteral:
            return "string";
        case Lexer::ComplexLiteral:
            return "complex";
        case Lexer::Assignment:
            return "assignment";
        case Lexer::Identifier:
            return "name";
        case Lexer::Operator:
            return "operator";
        case Lexer::OpenBracket:
            return "[";
        case Lexer::CloseBracket:
            return "]";
        case Lexer::OpenParenthesis:
            return "(";
        case Lexer::CloseParenthesis:
            return ")";
        case Lexer::OpenBrace:
            return "{";
        case Lexer::CloseBrace:
            return "}";
        case Lexer::OpenChevron:
            return "<";
        case Lexer::CloseChevron:
            return ">";
        case Lexer::Comma:
            return ",";
        case Lexer::Dot:
            return ".";
        case Lexer::Colon:
            return ":";
    }
}

CompilerError::CompilerError(std::string filename, int lineNumber, int column, std::string line) {
    m_filename = filename;
    m_lineNumber = lineNumber;
    m_column = column;
    m_line = line;
}

CompilerError::CompilerError(Lexer::Token *token) :
        CompilerError(token->filename, token->lineNumber, token->column, token->line) {

}

CompilerError::CompilerError(AST::Node *node) :
        CompilerError(node->token) {

}

void CompilerError::print() const {
    std::cout << m_prefix << " in " << m_filename << " on line " << m_lineNumber << " column " << m_column << std::endl;
    std::cout << std::endl;

    std::cout << "    " << m_line << std::endl;

    std::cout << "    ";
    for (int i = 0; i < m_column; i++) {
        std::cout << " ";
    }
    std::cout << "^" << std::endl;
    std::cout << std::endl;

    std::cout << m_message << std::endl;
}

SyntaxError::SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation) :
        CompilerError(filename, lineNumber, column, line) {
    m_message = "Got: " + got + "\nExpected: " + expectation;
}

SyntaxError::SyntaxError(Lexer::Token *token, std::string expectation) :
        CompilerError(token) {
    m_prefix = "Invalid syntax";

    std::string got = token->lexeme;

    if (got.empty()) {
        got = "(" + rule_string(token->rule) + ")";
    }

    makeMessage(got, expectation);
}

SyntaxError::SyntaxError(Lexer::Token *token, Lexer::Rule rule) :
        SyntaxError(token, rule_string(rule)) {

}

void SyntaxError::makeMessage(std::string got, std::string expectation) {
    std::stringstream ss;
    ss << "Got: " << got << "\n";
    ss << "Expected: " << expectation;
    m_message = ss.str();
}

UndefinedError::UndefinedError(AST::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Undefined error";
    m_message = name + " is not defined in this scope.";
}

RedefinedError::RedefinedError(AST::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Redefined error";
    m_message = name + " is already defined in this scope.";
}
