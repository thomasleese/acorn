//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include <unicode/unistr.h>
#include <boost/regex/icu.hpp>

#include "Errors.h"

#include "LexicalAnalysis.h"

using namespace LexicalAnalysis;

Lexer::Lexer() {
    loadRules();
}

Lexer::~Lexer() {

}

void Lexer::setRule(Token::Rule rule, std::string regex) {
    m_rules[rule] = "^" + regex;
}

void Lexer::loadRules() {
    setRule(Token::Whitespace, "([\t ]+)");
    setRule(Token::Newline, "([\r\n]+)");
    setRule(Token::Comment, "(#[^\n\r]+[\n\r]+)");

    std::string keywordSuffix = "(?:[\n\r ]+)";
    setRule(Token::LetKeyword, "(let)" + keywordSuffix);
    setRule(Token::DefKeyword, "(def)" + keywordSuffix);
    setRule(Token::TypeKeyword, "(type)" + keywordSuffix);
    setRule(Token::AsKeyword, "(as)" + keywordSuffix);
    setRule(Token::EndKeyword, "(end)" + keywordSuffix);
    setRule(Token::WhileKeyword, "(while)" + keywordSuffix);
    setRule(Token::ForKeyword, "(for)" + keywordSuffix);
    setRule(Token::InKeyword, "(in)" + keywordSuffix);
    setRule(Token::IfKeyword, "(if)" + keywordSuffix);
    setRule(Token::ElseKeyword, "(else)" + keywordSuffix);

    setRule(Token::BooleanLiteral, "(true|false)");
    setRule(Token::StringLiteral, "(\"(?:\\.|[^\"])*\")");
    setRule(Token::FloatLiteral, "([0-9]+\\.[0-9]+)");
    setRule(Token::ComplexLiteral, "([0-9]+(?:\\.[0-9])?\\+i[0-9]+(?:\\.[0-9]+)?)");
    setRule(Token::IntegerLiteral, "([0-9]+)");

    setRule(Token::OpenBracket, "(\\[)");
    setRule(Token::CloseBracket, "(\\])");
    setRule(Token::OpenParenthesis, "(\\()");
    setRule(Token::CloseParenthesis, "(\\))");
    setRule(Token::OpenBrace, "(\\{)");
    setRule(Token::CloseBrace, "(\\})");
    setRule(Token::OpenChevron, "(<)");
    setRule(Token::CloseChevron, "(>)");
    setRule(Token::Comma, "(,)");
    setRule(Token::Dot, "(\\.)");
    setRule(Token::Colon, "(:)");

    std::string nameInitialRegex = "[:L*:][:Nl:][:Sc:][:So:]√";
    std::string nameAfterRegex = nameInitialRegex + "![:N*:][:M*:][:Sk:][:Pc:]";
    setRule(Token::Assignment, "(=[^=])");
    setRule(Token::Identifier, "([" + nameInitialRegex + "][" + nameAfterRegex + "]*)");
    setRule(Token::Operator, "([:Sm:])");
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
            Token::Rule rule = it->first;
            boost::u32regex pattern = boost::make_u32regex(it->second);

            if (boost::u32regex_search(substr, matcher, pattern)) {
                std::string value = matcher.str(1);

                if (substr.substr(0, value.size()) == value) {
                    if (rule != Token::Whitespace && rule != Token::Comment) {
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

                    if (rule == Token::Newline) {
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
    endOfFileToken->rule = Token::EndOfFile;
    endOfFileToken->lexeme = "";

    endOfFileToken->filename = filename;
    endOfFileToken->line = currentLine;
    endOfFileToken->column = currentColumn;
    endOfFileToken->lineNumber = currentLineNumber;

    tokens.push_back(endOfFileToken);

    return tokens;
}
