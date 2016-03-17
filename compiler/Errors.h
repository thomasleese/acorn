//
// Created by Thomas Leese on 15/03/2016.
//

#include <exception>

#ifndef QUARK_ERRORS_H
#define QUARK_ERRORS_H

#include <string>

namespace AST {
    struct Node;
}

namespace LexicalAnalysis {
    struct Token;
}

namespace Errors {

    class CompilerError : public std::exception {
    public:
        explicit CompilerError(std::string filename, int lineNumber, int column, std::string line);
        CompilerError(LexicalAnalysis::Token *token);
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

    class SyntaxError : public CompilerError {
    public:
        explicit SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation);
        SyntaxError(LexicalAnalysis::Token *token, std::string expectation);
        SyntaxError(LexicalAnalysis::Token *token, LexicalAnalysis::Rule rule);

    private:
        void makeMessage(std::string got, std::string expectation);
    };

    class UndefinedError : public CompilerError {
    public:
        explicit UndefinedError(AST::Node *node, std::string name);
    };

    class RedefinedError : public CompilerError {
    public:
        explicit RedefinedError(AST::Node *node, std::string name);
    };

};

#endif //QUARK_ERRORS_H
