//
// Created by Thomas Leese on 12/01/2017.
//

#include "token.h"

using namespace acorn;

std::string Token::as_string(Token::Kind kind)
{
    switch (kind) {
        case EndOfFile:
            return "EOF";
        case Newline:
            return "newline";
        case Indent:
            return "indent";
        case Deindent:
            return "deindent";
        case LetKeyword:
            return "let";
        case DefKeyword:
            return "def";
        case TypeKeyword:
            return "type";
        case AsKeyword:
            return "as";
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
        case RepeatKeyword:
            return "repeat";
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
            return "protocol";
        case EnumKeyword:
            return "enum";
        case SwitchKeyword:
            return "switch";
        case CaseKeyword:
            return "case";
        case DefaultKeyword:
            return "default";
        case IntegerLiteral:
            return "integer";
        case FloatLiteral:
            return "float";
        case StringLiteral:
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
        case Arrow:
            return "->";
        case Assignment:
            return "=";
        case Name:
            return "name";
        case Operator:
            return "operator";
    }
}