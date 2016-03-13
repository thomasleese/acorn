//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>

#include "Lexer.h"

Lexer::Lexer() {
    loadRules();
}

Lexer::~Lexer() {

}

void Lexer::loadRules() {
    m_rules[Lexer::Whitespace] = "[\\t ]";
    m_rules[Lexer::Newline] = "\n";
    m_rules[Lexer::LetKeyword] = "let";

    m_rules[Lexer::Identifier] = "[[:alpha:]_][[:alpha:]|_]*";

    m_rules[Lexer::IntegerLiteral] = "[0-9]+";

    m_rules[Lexer::Arrow] = "->";

    m_rules[Lexer::OpenParenthesis] = "\\(";
    m_rules[Lexer::CloseParenthesis] = "\\)";
    m_rules[Lexer::OpenBrace] = "\\{";
    m_rules[Lexer::CloseBrace] = "\\}";

    m_rules[Lexer::Comma] = ",";
    m_rules[Lexer::Plus] = "\\+";
    m_rules[Lexer::Assignment] = "=";
}

std::vector<Lexer::Token> Lexer::tokenise(std::string filename) const {
    std::stringstream bufferStream;

    std::ifstream in(filename.c_str());
    while (in.good()) {
        std::string line;
        std::getline(in, line);

        bufferStream << line;
        bufferStream << "\n";
    }

    std::string buffer = bufferStream.str();

    int pos = 0;
    int end = buffer.length();

    std::smatch matcher;

    std::vector<Token> tokens;

    while (pos < end) {
        std::string substr = buffer.substr(pos, end - pos);

        int col = 0;
        bool found = false;

        for (auto it = m_rules.begin(); it != m_rules.end(); it++) {
            Rule rule = it->first;
            std::regex pattern(it->second, std::regex_constants::extended);

            if (std::regex_search(substr, matcher, pattern)) {
                std::string value = matcher[0];

                if (substr.substr(0, value.length()) == value) {
                    if (rule != Whitespace) {
                        Token token;
                        token.rule = rule;
                        token.lexeme = value;

                        tokens.push_back(token);
                    }

                    pos += value.length();
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::string token = "'" + substr.substr(0, 1) + "'";
            std::string msg = "Unknown character, skipping...";
            std::cout << msg << " " << token << std::endl;
            pos++;
        }
    }

    return tokens;
}