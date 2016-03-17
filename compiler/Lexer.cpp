//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include <unicode/unistr.h>
#include <boost/regex/icu.hpp>

#include "Errors.h"
#include "Lexer.h"

Lexer::Lexer() {
    loadRules();
}

Lexer::~Lexer() {

}

void Lexer::addRule(Rule rule, std::string regex) {
    m_rules[rule] = "^" + regex;
}

void Lexer::loadRules() {
    addRule(Lexer::Whitespace, "([\t ]+)");
    addRule(Lexer::Newline, "([\r\n]+)");
    addRule(Lexer::Comment, "(#[^\n\r]+[\n\r]+)");

    std::string keywordSuffix = "(?:[\n\r ]+)";
    addRule(Lexer::LetKeyword, "(let)" + keywordSuffix);
    addRule(Lexer::DefKeyword, "(def)" + keywordSuffix);
    addRule(Lexer::TypeKeyword, "(type)" + keywordSuffix);
    addRule(Lexer::AsKeyword, "(as)" + keywordSuffix);
    addRule(Lexer::EndKeyword, "(end)" + keywordSuffix);
    addRule(Lexer::WhileKeyword, "(while)" + keywordSuffix);
    addRule(Lexer::ForKeyword, "(for)" + keywordSuffix);
    addRule(Lexer::InKeyword, "(in)" + keywordSuffix);
    addRule(Lexer::IfKeyword, "(if)" + keywordSuffix);
    addRule(Lexer::ElseKeyword, "(else)" + keywordSuffix);

    addRule(Lexer::BooleanLiteral, "(true|false)");
    addRule(Lexer::StringLiteral, "(\"(?:\\.|[^\"])*\")");
    addRule(Lexer::FloatLiteral, "([0-9]+\\.[0-9]+)");
    addRule(Lexer::ComplexLiteral, "([0-9]+(?:\\.[0-9])?\\+i[0-9]+(?:\\.[0-9]+)?)");
    addRule(Lexer::IntegerLiteral, "([0-9]+)");

    addRule(Lexer::OpenBracket, "(\\[)");
    addRule(Lexer::CloseBracket, "(\\])");
    addRule(Lexer::OpenParenthesis, "(\\()");
    addRule(Lexer::CloseParenthesis, "(\\))");
    addRule(Lexer::OpenBrace, "(\\{)");
    addRule(Lexer::CloseBrace, "(\\})");
    addRule(Lexer::OpenChevron, "(<)");
    addRule(Lexer::CloseChevron, "(>)");
    addRule(Lexer::Comma, "(,)");
    addRule(Lexer::Dot, "(\\.)");
    addRule(Lexer::Colon, "(:)");

    std::string nameInitialRegex = "[:L*:][:Nl:][:Sc:][:So:]√";
    std::string nameAfterRegex = nameInitialRegex + "![:N*:][:M*:][:Sk:][:Pc:]";
    addRule(Lexer::Assignment, "(=[^=])");
    addRule(Lexer::Identifier, "([" + nameInitialRegex + "][" + nameAfterRegex + "]*)");
    addRule(Lexer::Operator, "([:Sm:])");
}

std::vector<Lexer::Token *> Lexer::tokenise(std::string filename) const {
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