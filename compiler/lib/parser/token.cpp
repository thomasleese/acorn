//
// Created by Thomas Leese on 12/01/2017.
//

#include "acorn/parser/token.h"

using namespace acorn;
using namespace acorn::parser;

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

std::string Token::to_string() const {
    if (kind == Keyword) {
        return lexeme;
    } else {
        return kind_to_string(kind);
    }
}

std::ostream &parser::operator<<(std::ostream &stream, const Token &token) {
    return stream << token.to_string();
}
