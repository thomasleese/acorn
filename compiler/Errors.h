//
// Created by Thomas Leese on 15/03/2016.
//

#include <exception>

#ifndef QUARK_ERRORS_H
#define QUARK_ERRORS_H

#include <string>

#include "Lexer.h"

namespace Errors {

    class CompilerError : public std::exception {
    public:
        explicit CompilerError(std::string filename);

        const char *what() const noexcept;

        virtual void print() const = 0;

    protected:
        std::string m_filename;
    };

    class SyntaxError : public CompilerError {
    public:
        explicit SyntaxError(std::string filename, int lineNumber, int column, std::string line, std::string got, std::string expectation);
        SyntaxError(Lexer::Token *token, std::string expectation);
        SyntaxError(Lexer::Token *token, Lexer::Rule rule);

        void print() const;

    private:
        int m_lineNumber;
        int m_column;
        std::string m_line;
        std::string m_got;
        std::string m_expectation;
    };

};

#endif //QUARK_ERRORS_H
