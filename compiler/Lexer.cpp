//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>

#include "Errors.h"
#include "Lexer.h"

Lexer::Lexer() {
    loadRules();
}

Lexer::~Lexer() {

}

void Lexer::addRule(Rule rule, std::string regex) {
    m_rules[rule] = std::regex(regex, std::regex_constants::extended);
}

void Lexer::loadRules() {
    addRule(Lexer::Whitespace, "[\t ]+");
    addRule(Lexer::Newline, "[\r\n]+");
    addRule(Lexer::Comment, "#[^\n\r]+");

    addRule(Lexer::LetKeyword, "let");
    addRule(Lexer::DefKeyword, "def");
    addRule(Lexer::TypeKeyword, "type");
    addRule(Lexer::AsKeyword, "as");
    addRule(Lexer::EndKeyword, "end");
    addRule(Lexer::WhileKeyword, "while");
    addRule(Lexer::ForKeyword, "for");
    addRule(Lexer::InKeyword, "in");
    addRule(Lexer::IfKeyword, "if");
    addRule(Lexer::ElseKeyword, "else");

    addRule(Lexer::BooleanLiteral, "true|false");
    addRule(Lexer::IntegerLiteral, "[0-9]+");
    addRule(Lexer::FloatLiteral, "[0-9]+\\.[0-9]+");
    addRule(Lexer::StringLiteral, "\"(\\.|[^\"])*\"");
    addRule(Lexer::ComplexLiteral, "([0-9]+)|([0-9]+\\.[0-9]+)i([0-9]+)|([0-9]+\\.[0-9]+)");

    addRule(Lexer::Operator, "\\+=|>=|<=|==|\\^|\\+|\\*|-|<|>");  // replace with unicode Sm
    addRule(Lexer::Identifier, "[[:alpha:]_][[:alpha:]_0-9]*");
    addRule(Lexer::Assignment, "=");

    addRule(Lexer::OpenBracket, "\\[");
    addRule(Lexer::CloseBracket, "]");
    addRule(Lexer::OpenParenthesis, "\\(");
    addRule(Lexer::CloseParenthesis, "\\)");
    addRule(Lexer::OpenBrace, "\\{");
    addRule(Lexer::CloseBrace, "\\}");
    addRule(Lexer::OpenChevron, "<");
    addRule(Lexer::CloseChevron, ">");
    addRule(Lexer::Comma, ",");
    addRule(Lexer::Dot, "\\.");
    addRule(Lexer::Colon, ":");
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

    int pos = 0;
    int end = buffer.length();

    std::smatch matcher;

    std::vector<Token *> tokens;

    while (pos < end) {
        std::string substr = buffer.substr(pos, end - pos);

        bool found = false;

        for (auto it = m_rules.begin(); it != m_rules.end(); it++) {
            Rule rule = it->first;
            std::regex pattern = it->second;

            if (std::regex_search(substr, matcher, pattern)) {
                std::string value = matcher[0];

                if (substr.substr(0, value.length()) == value) {
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

                    pos += value.length();

                    if (rule == Newline) {
                        currentLineNumber += value.length();  // each newline is a single character
                        currentColumn = 0;
                        currentLine = "";
                        std::getline(std::istringstream(buffer.substr(pos, pos - end)), currentLine);
                    } else {
                        currentColumn += value.length();
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