//
// Created by Thomas Leese on 15/03/2016.
//

#include <exception>

#ifndef JET_ERRORS_H
#define JET_ERRORS_H

#include <string>

#include "Token.h"

namespace jet {

    namespace ast {
        struct Node;
    }

    namespace errors {

        class CompilerError : public std::exception {
        public:
            explicit CompilerError(std::string filename, int lineNumber, int column, std::string line);

            CompilerError(Token *token);

            CompilerError(jet::ast::Node *node);

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

            FileNotFoundError(jet::ast::Node *node);
        };

        class InternalError : public CompilerError {
        public:
            InternalError(Token *token, std::string message);

            InternalError(jet::ast::Node *node, std::string message);
        };

        class InternalAstError : public InternalError {
        public:
            InternalAstError(Token *token);

            InternalAstError(jet::ast::Node *node);
        };

        class SyntaxError : public CompilerError {
        public:
            explicit SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got,
                                 std::string expectation);

            SyntaxError(Token *token, std::string expectation);

            SyntaxError(Token *token, Token::Rule rule);

        private:
            void makeMessage(std::string got, std::string expectation);
        };

        class UndefinedError : public CompilerError {
        public:
            explicit UndefinedError(jet::ast::Node *node, std::string name);
        };

        class TooManyDefinedError : public CompilerError {
        public:
            explicit TooManyDefinedError(jet::ast::Node *node, std::string name);
        };

        class RedefinedError : public CompilerError {
        public:
            explicit RedefinedError(jet::ast::Node *node, std::string name);
        };

        class InvalidTypeConstructor : public CompilerError {
        public:
            explicit InvalidTypeConstructor(jet::ast::Node *node);
        };

        class InvalidTypeParameters : public CompilerError {
        public:
            explicit InvalidTypeParameters(jet::ast::Node *node, unsigned long given_no, unsigned long expected_no);
        };

        class TypeMismatchError : public CompilerError {
        public:
            explicit TypeMismatchError(jet::ast::Node *node1, jet::ast::Node *node2);
        };

        class TypeInferenceError : public CompilerError {
        public:
            explicit TypeInferenceError(jet::ast::Node *node);
        };

        class ConstantAssignmentError : public CompilerError {
        public:
            ConstantAssignmentError(jet::ast::Node *node);
        };

    }

}

#endif // JET_ERRORS_H
