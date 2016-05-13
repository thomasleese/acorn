//
// Created by Thomas Leese on 15/03/2016.
//

#ifndef ACORN_ERRORS_H
#define ACORN_ERRORS_H

#include <exception>
#include <string>

#include "token.h"

namespace acorn {

    namespace ast {
        struct Node;
    }

    namespace errors {

        class CompilerError : public std::exception {
        public:
            explicit CompilerError(std::string filename, int lineNumber, int column, std::string line);
            CompilerError(Token *token);
            CompilerError(ast::Node *node);

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
            FileNotFoundError(ast::Node *node);
        };

        class InternalError : public CompilerError {
        public:
            InternalError(Token *token, std::string message);
            InternalError(ast::Node *node, std::string message);
        };

        class InternalAstError : public InternalError {
        public:
            InternalAstError(Token *token);
            InternalAstError(ast::Node *node);
        };

        class SyntaxError : public CompilerError {
        public:
            SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got,
                        std::string expectation);
            SyntaxError(Token *token, std::string expectation);
            SyntaxError(Token *token, Token::Rule rule);

        private:
            void makeMessage(std::string got, std::string expectation);
        };

        class UndefinedError : public CompilerError {
        public:
            UndefinedError(ast::Node *node, std::string name);
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
            TypeMismatchError(ast::Node *node1, ast::Node *node2);
        };

        class TypeInferenceError : public CompilerError {
        public:
            TypeInferenceError(ast::Node *node);
        };

        class ConstantAssignmentError : public CompilerError {
        public:
            ConstantAssignmentError(ast::Node *node);
        };

    }

}

#endif // ACORN_ERRORS_H
