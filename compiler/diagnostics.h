//
// Created by Thomas Leese on 15/03/2016.
//

#pragma once

#include <exception>
#include <string>

#include "token.h"

namespace acorn {

    namespace ast {
        class Node;
        class Expression;
    }

    namespace types {
        class Type;
    }

    namespace diagnostics {

        class CompilerError : public std::exception {
        public:
            explicit CompilerError(std::string filename, int lineNumber, int column, std::string line);
            CompilerError(const Token &token);
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
            FileNotFoundError(const Token &token);
            FileNotFoundError(ast::Node *node);
        };

        class InternalError : public CompilerError {
        public:
            InternalError(const Token &token, std::string message);
            InternalError(ast::Node *node, std::string message);
        };

        class InternalAstError : public InternalError {
        public:
            InternalAstError(const Token &token);
            InternalAstError(ast::Node *node);
        };

        class SyntaxError : public CompilerError {
        public:
            SyntaxError(std::string filename, int lineNumber, int column,
                        std::string line, std::string got,
                        std::string expectation);
            SyntaxError(const Token &token, std::string expectation);
            SyntaxError(const Token &token, Token::Kind kind);

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
            TypeMismatchError(ast::Expression *node1, ast::Expression *node2);
            TypeMismatchError(ast::Node *node, types::Type *type1, types::Type *type2);
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

            void debug(std::string line) const;
            void report(const CompilerError &error);

            bool has_errors() const;

        private:
            bool m_has_errors;

        };

    }

}
