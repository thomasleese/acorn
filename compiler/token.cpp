//
// Created by Thomas Leese on 17/03/2016.
//

#include "token.h"

using namespace acorn;

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
        case ThenKeyword:
            return "then";
        case ElseKeyword:
            return "else";
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
        case SpawnKeyword:
            return "spawn";
        case CCallKeyword:
            return "ccall";
        case UsingKeyword:
            return "using";
        case NewKeyword:
            return "new";
        case InoutKeyword:
            return "inout";
        case ProtocolKeyword:
            return "inout";
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
