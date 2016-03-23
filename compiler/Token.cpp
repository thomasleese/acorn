//
// Created by Thomas Leese on 17/03/2016.
//

#include "Token.h"

std::string Token::rule_string(Token::Rule rule) {
    switch (rule) {
        case Whitespace:
            return "whitespace";
        case Newline:
            return "new line";
        case Comment:
            return "comment";
        case EndOfFile:
            return "end of file";
        case LetKeyword:
            return "let";
        case DefKeyword:
            return "def";
        case TypeKeyword:
            return "type";
        case AsKeyword:
            return "as";
        case EndKeyword:
            return "end";
        case WhileKeyword:
            return "while";
        case ForKeyword:
            return "for";
        case InKeyword:
            return "in";
        case IfKeyword:
            return "if";
        case ElseKeyword:
            return "else";
        case AndKeyword:
            return "and";
        case OrKeyword:
            return "or";
        case NotKeyword:
            return "not";
        case ContinueKeyword:
            return "continue";
        case BreakKeyword:
            return "break";
        case TryKeyword:
            return "try";
        case ExceptKeyword:
            return "except";
        case RaiseKeyword:
            return "raise";
        case FinallyKeyword:
            return "finally";
        case FromKeyword:
            return "from";
        case ImportKeyword:
            return "import";
        case ReturnKeyword:
            return "return";
        case WithKeyword:
            return "with";
        case YieldKeyword:
            return "yield";
        case AsyncKeyword:
            return "async";
        case DoKeyword:
            return "do";
        case UnlessKeyword:
            return "unless";
        case MutableKeyword:
            return "mutable";
        case BooleanLiteral:
            return "boolean";
        case IntegerLiteral:
            return "integer";
        case FloatLiteral:
            return "float";
        case StringLiteral:
            return "string";
        case ImaginaryLiteral:
            return "imaginary";
        case Assignment:
            return "assignment";
        case Identifier:
            return "name";
        case Operator:
            return "operator";
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
        case OpenChevron:
            return "<";
        case CloseChevron:
            return ">";
        case Comma:
            return ",";
        case Dot:
            return ".";
        case Colon:
            return ":";
        case Semicolon:
            return ";";
    }
}
