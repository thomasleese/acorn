//
// Created by Thomas Leese on 15/03/2016.
//

#include <exception>

#ifndef JET_ERRORS_H
#define JET_ERRORS_H

#include <string>

#include "Token.h"

namespace AST {
    struct Node;
}

namespace Errors {

    class CompilerError : public std::exception {
    public:
        explicit CompilerError(std::string filename, int lineNumber, int column, std::string line);
        CompilerError(Token *token);
        CompilerError(AST::Node *node);

        void print() const;

    private:
        std::string m_filename;
        int m_lineNumber;
        int m_column;
        std::string m_line;

    protected:
        std::string m_prefix;
        std::string m_message;
    };

    class FileNotFoundError : public CompilerError {
    public:
        FileNotFoundError(Token *token);
        FileNotFoundError(AST::Node *node);
    };

    class InternalError : public CompilerError {
    public:
        InternalError(Token *token, std::string message);
        InternalError(AST::Node *node, std::string message);
    };

    class InternalAstError : public InternalError {
    public:
        InternalAstError(Token *token);
        InternalAstError(AST::Node *node);
    };

    class SyntaxError : public CompilerError {
    public:
        explicit SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation);
        SyntaxError(Token *token, std::string expectation);
        SyntaxError(Token *token, Token::Rule rule);

    private:
        void makeMessage(std::string got, std::string expectation);
    };

    class UndefinedError : public CompilerError {
    public:
        explicit UndefinedError(AST::Node *node, std::string name);
    };

    class TooManyDefinedError : public CompilerError {
    public:
        explicit TooManyDefinedError(AST::Node *node, std::string name);
    };

    class RedefinedError : public CompilerError {
    public:
        explicit RedefinedError(AST::Node *node, std::string name);
    };

    class InvalidTypeConstructor : public CompilerError {
    public:
        explicit InvalidTypeConstructor(AST::Node *node);
    };

    class InvalidTypeParameters : public CompilerError {
    public:
        explicit InvalidTypeParameters(AST::Node *node);
    };

    class TypeMismatchError : public CompilerError {
    public:
        explicit TypeMismatchError(AST::Node *node1, AST::Node *node2);
    };

    class TypeInferenceError : public CompilerError {
    public:
        explicit TypeInferenceError(AST::Node *node);
    };

};

#endif // JET_ERRORS_H
