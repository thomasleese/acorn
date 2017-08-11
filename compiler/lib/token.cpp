//
// Created by Thomas Leese on 12/01/2017.
//

#include "acorn/token.h"

using namespace acorn;

std::string Token::as_string(Token::Kind kind)
{
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
