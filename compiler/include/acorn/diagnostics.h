//
// Created by Thomas Leese on 15/03/2016.
//

#pragma once

#include <exception>
#include <string>

#include "parser/token.h"

namespace acorn {

    namespace ast {
        class Node;
        class Expression;
        class Name;
    }

    namespace typesystem {
        class Type;
    }

    using parser::SourceLocation;
    using parser::Token;

}

namespace acorn::diagnostics {

    class CompilerError {
    public:
        explicit CompilerError(const SourceLocation &location);
        explicit CompilerError(const Token &token);
        explicit CompilerError(ast::Node *node);

        void print() const;

    private:
        parser::SourceLocation m_location;

    protected:
        std::string m_prefix;
        std::string m_message;
    };

    class FileNotFoundError : public CompilerError {
    public:
        FileNotFoundError(const Token &token);
        FileNotFoundError(ast::Node *node);
    };

    class SyntaxError : public CompilerError {
    public:
        SyntaxError(const parser::SourceLocation &location, std::string got,
                    std::string expectation);
        SyntaxError(const Token &token, std::string expectation);
        SyntaxError(const Token &token, Token::Kind kind);

    private:
        void makeMessage(std::string got, std::string expectation);
    };

    class UndefinedError : public CompilerError {
    public:
        UndefinedError(ast::Node *node, std::string message);
        explicit UndefinedError(ast::Name *name);
    };

    class TooManyDefinedError : public CompilerError {
    public:
        TooManyDefinedError(ast::Node *node, std::string name);
    };

    class RedefinedError : public CompilerError {
    public:
        RedefinedError(ast::Node *node, std::string name);
    };

    class InvalidTypeConstructor : public CompilerError {
    public:
        InvalidTypeConstructor(ast::Node *node);
    };

    class InvalidTypeParameters : public CompilerError {
    public:
        InvalidTypeParameters(ast::Node *node, unsigned long given_no, unsigned long expected_no);
    };

    class TypeMismatchError : public CompilerError {
    public:
        TypeMismatchError(ast::Expression *node1, ast::Expression *node2);
        TypeMismatchError(ast::Node *node, typesystem::Type *type1, typesystem::Type *type2);
        TypeMismatchError(ast::Node *node, std::string type1, std::string type2);
    };

    class TypeInferenceError : public CompilerError {
    public:
        TypeInferenceError(ast::Node *node);
    };

    class ConstantAssignmentError : public CompilerError {
    public:
        ConstantAssignmentError(ast::Node *node);
    };

    class Reporter {

    public:
        Reporter();

        void report(const CompilerError &error);

        bool has_errors() const;

    private:
        bool m_has_errors;

    };

}
