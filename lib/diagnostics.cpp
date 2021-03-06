#include <iostream>
#include <sstream>

#include "acorn/ast/nodes.h"
#include "acorn/typesystem/types.h"

#include "acorn/diagnostics.h"

using namespace acorn;
using namespace acorn::diagnostics;
using namespace acorn::parser;

CompilerError::CompilerError(const SourceLocation &location) : m_location(location) { }

CompilerError::CompilerError(const Token &token) : CompilerError(token.location) { }

CompilerError::CompilerError(ast::Node *node) : CompilerError(node->token()) { }

std::ostream& diagnostics::operator<<(std::ostream& os, const CompilerError &error) {
    return os
        << error.m_prefix << " in " << error.m_location << std::endl << std::endl
        << "    " << error.m_location.line << std::endl
        << std::string(error.m_location.column + 3, ' ') << '^' << std::endl << std::endl
        << error.m_message;
}

FileNotFoundError::FileNotFoundError(const Token &token, const char *filename) : CompilerError(token) {
    m_prefix = "File not found";
    m_message = filename;
}

FileNotFoundError::FileNotFoundError(ast::Node *node, const char *filename) : FileNotFoundError(node->token(), filename) { }

SyntaxError::SyntaxError(const SourceLocation &location, std::string got, std::string expectation)
    : CompilerError(location) {
    m_prefix = "Invalid syntax";
    m_message = "Got: " + got + "\nExpected: " + expectation;
}

SyntaxError::SyntaxError(const Token &token, std::string expectation) : CompilerError(token) {
    m_prefix = "Invalid syntax";

    if (token.lexeme.empty()) {
        std::stringstream ss;
        ss << '(' << token << ')';
        make_message(ss.str(), expectation);
    } else {
        make_message(token.lexeme, expectation);
    }
}

SyntaxError::SyntaxError(const Token &token, Token::Kind kind) :
    SyntaxError(token, Token::kind_to_string(kind)) { }

void SyntaxError::make_message(std::string got, std::string expectation) {
    std::stringstream ss;
    ss << "Got: " << got << "\n";
    ss << "Expected: " << expectation;
    m_message = ss.str();
}

UndefinedError::UndefinedError(ast::Node *node, std::string message) : CompilerError(node) {
    m_prefix = "Undefined error";
    m_message = message;
}

UndefinedError::UndefinedError(ast::Name *name)
    : UndefinedError(name, name->value() + " is not defined in scope.") { }

UndefinedError::UndefinedError(ast::ParamName *name)
    : UndefinedError(name->name()) { }

TooManyDefinedError::TooManyDefinedError(ast::Node *node, std::string name)
    : CompilerError(node) {
    m_prefix = "Too many defined error";
    m_message = name + " has multiple definitions.";
}

RedefinedError::RedefinedError(ast::Node *node, std::string name)
    : CompilerError(node) {
    m_prefix = "Redefined error";
    m_message = name + " is already defined in this scope.";
}

InvalidTypeConstructor::InvalidTypeConstructor(ast::Node *node) : CompilerError(node) {
    m_prefix = "Invalid type";
    m_message = "This is not a type type.";
}

InvalidTypeParameters::InvalidTypeParameters(ast::Node *node, unsigned long given_no, unsigned long expected_no)
    : CompilerError(node) {
    m_prefix = "Invalid type parameters";

    std::stringstream ss;
    ss << "Got " << given_no << " parameters, but expected " << expected_no << ".";
    m_message = ss.str();
}

TypeMismatchError::TypeMismatchError(ast::Node *node1, ast::Node *node2)
    : TypeMismatchError(node1, node1->type(), node2 ? node2->type() : nullptr) { }

TypeMismatchError::TypeMismatchError(ast::Node *node, typesystem::Type *type1, typesystem::Type *type2)
    : TypeMismatchError(node, type1->name(), type2->name()) { }

TypeMismatchError::TypeMismatchError(ast::Node *node, std::string type1, std::string type2)
    : CompilerError(node) {
    m_prefix = "Invalid typesystem";

    std::stringstream ss;
    ss << "Got: " << type1 << "\n";
    ss << "Expected: " << type2;
    m_message = ss.str();
}

TypeInferenceError::TypeInferenceError(ast::Node *node)
    : CompilerError(node) {
    m_prefix = "Type inference error";
    m_message = "Try specifying the type you want.";
}

ConstantAssignmentError::ConstantAssignmentError(ast::Node *node)
    : CompilerError(node) {
    m_prefix = "Assignment to constant";
    m_message = "Variable is not mutable.";
}

Logger::Logger(const char *name) {
    if (name == nullptr) {
        name = "acorn";
    }

    auto spdlog = spdlog::get(name);
    if (spdlog == nullptr) {
        spdlog = spdlog::stdout_color_mt(name);

        spdlog->set_level(spdlog::level::trace);
        spdlog->set_pattern("[%H:%M:%S] %L %n - %v");

        spdlog->info("initialising logger");
    }

    assert(spdlog != nullptr);

    m_spdlog = spdlog;
}

Reporter::Reporter() : m_has_errors(false) { }

void Reporter::report(const CompilerError &error) {
    std::cerr << error << std::endl;
    m_has_errors = true;
}
