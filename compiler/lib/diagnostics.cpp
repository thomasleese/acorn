//
// Created by Thomas Leese on 15/03/2016.
//

#include <iostream>
#include <sstream>

#include "acorn/ast/nodes.h"
#include "acorn/lexer.h"
#include "acorn/typesystem/types.h"

#include "acorn/diagnostics.h"

using namespace acorn;
using namespace acorn::diagnostics;

CompilerError::CompilerError(std::string filename, int lineNumber, int column, std::string line) : m_filename(filename), m_lineNumber(lineNumber), m_column(column), m_line(line) {

}

CompilerError::CompilerError(const Token &token) : CompilerError(token.filename, token.line_number, token.column, token.line) {

}

CompilerError::CompilerError(ast::Node *node) : CompilerError(node->token()) {

}

void CompilerError::print() const {
    std::cout << m_prefix << " in " << m_filename << " on line " << m_lineNumber << " column " << m_column << std::endl;
    std::cout << std::endl;

    std::cout << "    " << m_line << std::endl;

    std::cout << "    ";
    for (int i = 1; i < m_column; i++) {
        std::cout << " ";
    }
    std::cout << "^" << std::endl;
    std::cout << std::endl;

    std::cout << m_message << std::endl;
}

FileNotFoundError::FileNotFoundError(const Token &token) : CompilerError(token) {
    m_prefix = "File not found";
}

FileNotFoundError::FileNotFoundError(ast::Node *node) : CompilerError(node) {
    m_prefix = "File not found";
}

InternalError::InternalError(const Token &token, std::string message) : CompilerError(token) {
    m_prefix = "Internal error";
    m_message = message + "\nNote: You have probably encountered a bug in Acorn, not your code.";
}

InternalError::InternalError(ast::Node *node, std::string message) : CompilerError(node) {
    m_prefix = "Internal error";
    m_message = message + "\nNote: You have probably encountered a bug in Acorn, not your code.";
}

InternalAstError::InternalAstError(const Token &token) :
        InternalError(token, "Should not be in lowered AST.") {

}

InternalAstError::InternalAstError(ast::Node *node) :
        InternalError(node, "Should not be in lowered AST.") {

}

SyntaxError::SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation) :
        CompilerError(filename, lineNumber, column, line) {
    m_prefix = "Invalid syntax";

    m_message = "Got: " + got + "\nExpected: " + expectation;
}

SyntaxError::SyntaxError(const Token &token, std::string expectation) :
        CompilerError(token) {
    m_prefix = "Invalid syntax";

    std::string got = token.lexeme;

    if (got.empty()) {
        got = "(" + token.to_string() + ")";
    }

    makeMessage(got, expectation);
}

SyntaxError::SyntaxError(const Token &token, Token::Kind kind) :
        SyntaxError(token, Token::kind_to_string(kind)) {

}

void SyntaxError::makeMessage(std::string got, std::string expectation) {
    std::stringstream ss;
    ss << "Got: " << got << "\n";
    ss << "Expected: " << expectation;
    m_message = ss.str();
}

UndefinedError::UndefinedError(ast::Node *node, std::string message) : CompilerError(node) {
    m_prefix = "Undefined error";
    m_message = message;
}

UndefinedError::UndefinedError(ast::Name *name) : UndefinedError(name, name->value() + " is not defined in scope.") {

}

TooManyDefinedError::TooManyDefinedError(ast::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Too many defined error";
    m_message = name + " has multiple definitions.";
}

RedefinedError::RedefinedError(ast::Node *node, std::string name) :
        CompilerError(node) {
    m_prefix = "Redefined error";
    m_message = name + " is already defined in this scope.";
}

InvalidTypeConstructor::InvalidTypeConstructor(ast::Node *node) : CompilerError(node) {
    m_prefix = "Invalid type";
    m_message = "This is not a type type.";
}

InvalidTypeParameters::InvalidTypeParameters(ast::Node *node, unsigned long given_no, unsigned long expected_no) :
        CompilerError(node) {
    m_prefix = "Invalid type parameters";

    std::stringstream ss;
    ss << "Got " << given_no << " parameters, but expected " << expected_no << ".";
    m_message = ss.str();
}

TypeMismatchError::TypeMismatchError(ast::Expression *node1, ast::Expression *node2) : TypeMismatchError(node1, node1->type(), node2 ? node2->type() : nullptr) {

}

TypeMismatchError::TypeMismatchError(ast::Node *node, typesystem::Type *type1, typesystem::Type *type2) : TypeMismatchError(node, type1->name(), type2->name()) {

}

TypeMismatchError::TypeMismatchError(ast::Node *node, std::string type1, std::string type2) : CompilerError(node) {
    m_prefix = "Invalid typesystem";

    std::stringstream ss;
    ss << "Got: " << type1 << "\n";
    ss << "Expected: " << type2;
    m_message = ss.str();
}

TypeInferenceError::TypeInferenceError(ast::Node *node) :
            CompilerError(node) {
    m_prefix = "Type inference error";
    m_message = "Try specifying the type you want.";
}

ConstantAssignmentError::ConstantAssignmentError(ast::Node *node) :
        CompilerError(node) {
    m_prefix = "Assignment to constant";
    m_message = "Variable is not mutable.";
}

Reporter::Reporter() : m_has_errors(false) {

}

void Reporter::debug(std::string line) const {
    std::cerr << line << std::endl;
}

void Reporter::report(const CompilerError &error) {
    error.print();
    m_has_errors = true;
}

bool Reporter::has_errors() const {
    return m_has_errors;
}