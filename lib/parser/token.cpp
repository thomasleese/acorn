//
// Created by Thomas Leese on 12/01/2017.
//

#include <sstream>

#include "acorn/parser/token.h"

using namespace acorn;
using namespace acorn::parser;

std::string SourceLocation::to_string() const {
    std::stringstream ss;
    ss << filename << "[" << line_number << "," << column << "]";
    return ss.str();
}

std::ostream &parser::operator<<(std::ostream &stream, const SourceLocation &location) {
    return stream
        << location.filename
        << "[" << location.line_number << "," << location.column << "]";
}

std::string Token::kind_to_string(const Kind &kind) {
    switch (kind) {
        case EndOfFile:
            return "EOF";
        case Newline:
            return "newline";
        case Indent:
            return "(indent)";
        case Deindent:
            return "(deindent)";
        case Keyword:
            return "keyword";
        case Int:
            return "integer";
        case Float:
            return "float";
        case String:
            return "string";
        case OpenBracket:
            return "[";
        case CloseBracket:
            return "]";
        case OpenParenthesis:
            return "(";
        case CloseParenthesis:
            return ")";
        case OpenBrace:
            return "{";
        case CloseBrace:
            return "}";
        case Comma:
            return ",";
        case Dot:
            return ".";
        case Colon:
            return ":";
        case Semicolon:
            return ";";
        case Assignment:
            return "=";
        case Name:
            return "name";
        case Operator:
            return "operator";
    }
}

std::string Token::kind_string() const {
    if (kind == Keyword) {
        return lexeme;
    } else {
        return kind_to_string(kind);
    }
}

std::string Token::lexeme_string() const {
    if (lexeme == "\n") {
        return "\\n";
    } else {
        return lexeme;
    }
}

std::string Token::to_string() const {
    std::ostringstream ss;
    ss << "Token(" << kind_string()
        << " '" << lexeme_string() << "'"
        << " " << location << ")";
    return ss.str();
}

std::ostream &parser::operator<<(std::ostream &stream, const Token &token) {
    return stream << "Token(" << token.kind_string()
        << " '" << token.lexeme_string() << "'"
        << " " << token.location << ")";
}
