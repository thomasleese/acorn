//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "AbstractSyntaxTree.h"
#include "Lexer.h"

#include "Errors.h"

using namespace Errors;

CompilerError::CompilerError(std::string filename, int lineNumber, int column, std::string line) {
    m_filename = filename;
    m_lineNumber = lineNumber;
    m_column = column;
    m_line = line;
}

CompilerError::CompilerError(Token *token) :
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

SyntaxError::SyntaxError(Token *token, std::string expectation) :
        CompilerError(token) {
    m_prefix = "Invalid syntax";

    std::string got = token->lexeme;

    if (got.empty()) {
        got = "(" + Token::rule_string(token->rule) + ")";
    }

    makeMessage(got, expectation);
}

SyntaxError::SyntaxError(Token *token, Token::Rule rule) :
        SyntaxError(token, Token::rule_string(rule)) {

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
