//
// Created by Thomas Leese on 12/01/2017.
//

#include <set>
#include <sstream>

#include "acorn/parser/token.h"

using namespace std;

using namespace acorn;
using namespace acorn::parser;

static set<string> keywords = {
    "let", "def", "type", "as", "while", "for", "in", "if", "else", "not",
    "and", "or", "end", "continue", "break", "try", "except", "raise",
    "finally", "from", "import", "return", "with", "yield", "async", "await",
    "repeat", "unless", "mutable", "spawn", "ccall", "using", "new", "inout",
    "protocol", "enum", "switch", "case", "default", "module", "builtin",
    "class", "interface", "static", "public", "private", "protected", "goto",
    "global", "virtual", "pass", "assert", "del",
};

bool acorn::parser::is_keyword(const string &name) {
    return keywords.find(name) != keywords.end();
}

SourceLocation::SourceLocation() : line_number(0), column(0) {

}

string SourceLocation::to_string() const {
    stringstream ss;
    ss << filename << "[" << line_number << "," << column << "]";
    return ss.str();
}

ostream &parser::operator<<(ostream &stream, const SourceLocation &location) {
    return stream << location.to_string();
}

Token::Token() {

}

Token::Token(Kind kind) : kind(kind) {

}

Token::Token(Kind kind, std::string lexeme) : kind(kind), lexeme(lexeme) {

}

string Token::kind_to_string(const Kind &kind) {
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

string Token::kind_string() const {
    if (kind == Keyword) {
        return lexeme;
    } else {
        return kind_to_string(kind);
    }
}

string Token::lexeme_string() const {
    if (lexeme == "\n") {
        return "\\n";
    } else {
        return lexeme;
    }
}

string Token::to_string() const {
    ostringstream ss;
    ss << "Token(" << kind_string()
        << " '" << lexeme_string() << "'"
        << " " << location << ")";
    return ss.str();
}

bool Token::operator==(const Token &token) {
    return kind == token.kind && lexeme == token.lexeme;
}

bool Token::operator!=(const Token &token) {
    return !(*this == token);
}

ostream &parser::operator<<(ostream &stream, const Token &token) {
    return stream << "Token(" << token.to_string();
}
