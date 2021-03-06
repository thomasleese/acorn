#pragma once

#include <exception>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "parser/token.h"

namespace acorn {

    namespace ast {
        class Node;
        class Name;
        class ParamName;
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

    private:
        friend std::ostream& operator<<(std::ostream& os, const CompilerError &error);

        parser::SourceLocation m_location;

    protected:
        std::string m_prefix;
        std::string m_message;
    };

    class FileNotFoundError : public CompilerError {
    public:
        FileNotFoundError(const Token &token, const char *filename);
        FileNotFoundError(ast::Node *node, const char *filename);
    };

    class SyntaxError : public CompilerError {
    public:
        SyntaxError(const parser::SourceLocation &location, std::string got,
                    std::string expectation);
        SyntaxError(const Token &token, std::string expectation);
        SyntaxError(const Token &token, Token::Kind kind);

    private:
        void make_message(std::string got, std::string expectation);
    };

    class UndefinedError : public CompilerError {
    public:
        UndefinedError(ast::Node *node, std::string message);
        explicit UndefinedError(ast::Name *name);
        explicit UndefinedError(ast::ParamName *name);
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

    class Logger {
    public:
        Logger(const char *name = nullptr);

        template <typename Arg1, typename... Args> void trace(const char *fmt, const Arg1 &arg1, const Args &... args) {
            m_spdlog->trace(fmt, arg1, args...);
        }

        template <typename Arg1, typename... Args> void debug(const char *fmt, const Arg1 &arg1, const Args &... args) {
            m_spdlog->debug(fmt, arg1, args...);
        }

        template <typename Arg1, typename... Args> void info(const char *fmt, const Arg1 &arg1, const Args &... args) {
            m_spdlog->info(fmt, arg1, args...);
        }

        template <typename Arg1, typename... Args> void warn(const char *fmt, const Arg1 &arg1, const Args &... args) {
            m_spdlog->warn(fmt, arg1, args...);
        }

        template <typename Arg1, typename... Args> void error(const char *fmt, const Arg1 &arg1, const Args &... args) {
            m_spdlog->error(fmt, arg1, args...);
        }

        template <typename Arg1, typename... Args> void critical(const char *fmt, const Arg1 &arg1, const Args &... args) {
            m_spdlog->critical(fmt, arg1, args...);
        }

        template <typename T> void trace(const T &msg) { m_spdlog->trace(msg); }
        template <typename T> void debug(const T &msg) { m_spdlog->debug(msg); }
        template <typename T> void info(const T &msg) { m_spdlog->info(msg); }
        template <typename T> void warn(const T &msg) { m_spdlog->warn(msg); }
        template <typename T> void error(const T &msg) { m_spdlog->error(msg); }
        template <typename T> void critical(const T &msg) { m_spdlog->critical(msg); }

    protected:
        std::shared_ptr<spdlog::logger> m_spdlog;
    };

    class Reporter {
    public:
        Reporter();

        void report(const CompilerError &error);

        bool has_errors() const { return m_has_errors; }

    private:
        bool m_has_errors;
    };

}
