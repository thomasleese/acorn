//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include <unicode/unistr.h>
#include <boost/regex/icu.hpp>

#include "LexicalAnalysis.h"

#include "Errors.h"

using namespace LexicalAnalysis;

std::string LexicalAnalysis::rule_string(Rule rule) {
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
        case BooleanLiteral:
            return "boolean";
        case IntegerLiteral:
            return "integer";
        case FloatLiteral:
            return "float";
        case StringLiteral:
            return "string";
        case ComplexLiteral:
            return "complex";
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
    }
}

Lexer::Lexer() {
    loadRules();
}

Lexer::~Lexer() {

}

void Lexer::setRule(Rule rule, std::string regex) {
    m_rules[rule] = "^" + regex;
}

void Lexer::loadRules() {
    setRule(Whitespace, "([\t ]+)");
    setRule(Newline, "([\r\n]+)");
    setRule(Comment, "(#[^\n\r]+[\n\r]+)");

    std::string keywordSuffix = "(?:[\n\r ]+)";
    setRule(LetKeyword, "(let)" + keywordSuffix);
    setRule(DefKeyword, "(def)" + keywordSuffix);
    setRule(TypeKeyword, "(type)" + keywordSuffix);
    setRule(AsKeyword, "(as)" + keywordSuffix);
    setRule(EndKeyword, "(end)" + keywordSuffix);
    setRule(WhileKeyword, "(while)" + keywordSuffix);
    setRule(ForKeyword, "(for)" + keywordSuffix);
    setRule(InKeyword, "(in)" + keywordSuffix);
    setRule(IfKeyword, "(if)" + keywordSuffix);
    setRule(ElseKeyword, "(else)" + keywordSuffix);

    setRule(BooleanLiteral, "(true|false)");
    setRule(StringLiteral, "(\"(?:\\.|[^\"])*\")");
    setRule(FloatLiteral, "([0-9]+\\.[0-9]+)");
    setRule(ComplexLiteral, "([0-9]+(?:\\.[0-9])?\\+i[0-9]+(?:\\.[0-9]+)?)");
    setRule(IntegerLiteral, "([0-9]+)");

    setRule(OpenBracket, "(\\[)");
    setRule(CloseBracket, "(\\])");
    setRule(OpenParenthesis, "(\\()");
    setRule(CloseParenthesis, "(\\))");
    setRule(OpenBrace, "(\\{)");
    setRule(CloseBrace, "(\\})");
    setRule(OpenChevron, "(<)");
    setRule(CloseChevron, "(>)");
    setRule(Comma, "(,)");
    setRule(Dot, "(\\.)");
    setRule(Colon, "(:)");

    std::string nameInitialRegex = "[:L*:][:Nl:][:Sc:][:So:]√";
    std::string nameAfterRegex = nameInitialRegex + "![:N*:][:M*:][:Sk:][:Pc:]";
    setRule(Assignment, "(=[^=])");
    setRule(Identifier, "([" + nameInitialRegex + "][" + nameAfterRegex + "]*)");
    setRule(Operator, "([:Sm:])");
}

std::vector<Token *> Lexer::tokenise(std::string filename) const {
    std::stringstream bufferStream;

    std::ifstream in(filename.c_str());
    while (in.good()) {
        std::string line;
        std::getline(in, line);

        bufferStream << line;
        bufferStream << "\n";
    }

    std::string buffer = bufferStream.str();

    int currentLineNumber = 1;
    int currentColumn = 0;
    std::string currentLine;

    std::getline(std::istringstream(buffer), currentLine);

    unsigned long pos = 0;
    unsigned long end = buffer.length();

    boost::smatch matcher;

    std::vector<Token *> tokens;

    while (pos < end) {
        std::string substr = buffer.substr(pos, end - pos);

        bool found = false;

        for (auto it = m_rules.begin(); it != m_rules.end(); it++) {
            Rule rule = it->first;
            boost::u32regex pattern = boost::make_u32regex(it->second);

            if (boost::u32regex_search(substr, matcher, pattern)) {
                std::string value = matcher.str(1);

                if (substr.substr(0, value.size()) == value) {
                    if (rule != Whitespace && rule != Comment) {
                        Token *token = new Token();
                        token->rule = rule;
                        token->lexeme = value;

                        token->filename = filename;
                        token->line = currentLine;
                        token->column = currentColumn;
                        token->lineNumber = currentLineNumber;

                        tokens.push_back(token);
                    }

                    pos += value.size();

                    if (rule == Newline) {
                        currentLineNumber += value.size();  // each newline is a single character
                        currentColumn = 0;
                        currentLine = "";
                        std::getline(std::istringstream(buffer.substr(pos, pos - end)), currentLine);
                    } else {
                        icu::UnicodeString s = icu::UnicodeString::fromUTF8(value);
                        currentColumn += s.countChar32();
                    }

                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::string token = substr.substr(0, 1);
            throw Errors::SyntaxError(filename, currentLineNumber, currentColumn, currentLine, token, "code");
        }
    }

    Token *endOfFileToken = new Token();
    endOfFileToken->rule = EndOfFile;
    endOfFileToken->lexeme = "";

    endOfFileToken->filename = filename;
    endOfFileToken->line = currentLine;
    endOfFileToken->column = currentColumn;
    endOfFileToken->lineNumber = currentLineNumber;

    tokens.push_back(endOfFileToken);

    return tokens;
}