//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "ast/nodes.h"
#include "Lexer.h"
#include "Types.h"

#include "errors.h"

using namespace jet;
using namespace jet::errors;

CompilerError::CompilerError(std::string filename, int lineNumber, int column, std::string line) {
    m_filename = filename;
    m_lineNumber = lineNumber;
    m_column = column;
    m_line = line;
}

CompilerError::CompilerError(Token *token) :
        CompilerError(token->filename, token->lineNumber, token->column, token->line) {

}

CompilerError::CompilerError(jet::ast::Node *node) :
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

FileNotFoundError::FileNotFoundError(Token *token) : CompilerError(token) {
    m_prefix = "File not found";
}

FileNotFoundError::FileNotFoundError(jet::ast::Node *node) : CompilerError(node) {
    m_prefix = "File not found";
}

InternalError::InternalError(Token *token, std::string message) :
        CompilerError(token) {
    m_prefix = "Internal error";
    m_message = message + "\nNote: You have probably encountered a bug in Jet, not your code.";
}

InternalError::InternalError(jet::ast::Node *node, std::string message) :
        CompilerError(node) {
    m_prefix = "Internal error";
    m_message = message + "\nNote: You have probably encountered a bug in Jet, not your code.";
}

InternalAstError::InternalAstError(Token *token) :
        InternalError(token, "Should not be in lowered AST.") {

}

InternalAstError::InternalAstError(jet::ast::Node *node) :
        InternalError(node, "Should not be in lowered AST.") {

}

SyntaxError::SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation) :
        CompilerError(filename, lineNumber, column, line) {
    m_prefix = "Invalid syntax";

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

UndefinedError::UndefinedError(jet::ast::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Undefined error";
    m_message = name + " is not defined in this scope.";
}

TooManyDefinedError::TooManyDefinedError(jet::ast::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Too many defined error";
    m_message = name + " has multiple definitions.";
}

RedefinedError::RedefinedError(jet::ast::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Redefined error";
    m_message = name + " is already defined in this scope.";
}

InvalidTypeConstructor::InvalidTypeConstructor(jet::ast::Node *node) :
        CompilerError(node) {
    m_prefix = "Invalid type";
    m_message = "This is not a type constructor.";
}

InvalidTypeParameters::InvalidTypeParameters(jet::ast::Node *node, unsigned long given_no, unsigned long expected_no) :
        CompilerError(node) {
    m_prefix = "Invalid type parameters";

    std::stringstream ss;
    ss << "Got " << given_no << " parameters, but expected " << expected_no << ".";
    m_message = ss.str();
}

TypeMismatchError::TypeMismatchError(jet::ast::Node *node1, jet::ast::Node *node2) :
        CompilerError(node1) {
    m_prefix = "Invalid types";

    std::stringstream ss;
    ss << "Got: " << node1->type->name() << "\n";
    ss << "Expected: " << node2->type->name();
    m_message = ss.str();
}

TypeInferenceError::TypeInferenceError(jet::ast::Node *node) :
            CompilerError(node) {
    m_prefix = "Type inference error";
    m_message = "Try specifying the type you want.";
}

ConstantAssignmentError::ConstantAssignmentError(jet::ast::Node *node) :
        CompilerError(node) {
    m_prefix = "Assignment to constant";
    m_message = "Variable is not mutable.";
}
