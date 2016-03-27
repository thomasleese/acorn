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

InternalError::InternalError(Token *token, std::string message) :
        CompilerError(token) {
    m_prefix = "Internal error";
    m_message = message + "\nNote: You have probably encountered a bug in Jet, not your code.";
}

InternalError::InternalError(AST::Node *node, std::string message) :
        CompilerError(node) {
    m_prefix = "Internal error";
    m_message = message + "\nNote: You have probably encountered a bug in Jet, not your code.";
}

InternalAstError::InternalAstError(Token *token) :
        InternalError(token, "Should not be in lowered AST.") {

}

InternalAstError::InternalAstError(AST::Node *node) :
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

UndefinedError::UndefinedError(AST::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Undefined error";
    m_message = name + " is not defined in this scope.";
}

TooManyDefinedError::TooManyDefinedError(AST::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Too many defined error";
    m_message = name + " has multiple definitions.";
}

RedefinedError::RedefinedError(AST::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Redefined error";
    m_message = name + " is already defined in this scope.";
}

InvalidTypeConstructor::InvalidTypeConstructor(AST::Node *node) :
        CompilerError(node) {
    m_prefix = "Invalid type";
    m_message = "This is not a type constructor.";
}

InvalidTypeParameters::InvalidTypeParameters(AST::Node *node) :
        CompilerError(node) {
    m_prefix = "Invalid type parameters";
    m_message = "Invalid type parmeters for this type.";
}

TypeMismatchError::TypeMismatchError(AST::Node *node1, AST::Node *node2) :
        CompilerError(node1) {
    m_prefix = "Invalid types";

    std::stringstream ss;
    ss << "Got: " << node2->type->name() << "\n";
    ss << "Expected: " << node1->type->name();
    m_message = ss.str();
}

TypeInferenceError::TypeInferenceError(AST::Node *node) :
            CompilerError(node) {
    m_prefix = "Type inference error";
    m_message = "Try specifying the type you want.";
}
