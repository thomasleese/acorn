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

using namespace jet;

Lexer::Lexer() {
    loadRules();
}

Lexer::~Lexer() {

}

void Lexer::setRule(Token::Rule rule, std::string regex) {
    m_rules[rule] = "^" + regex;
}

void Lexer::setKeywordRule(Token::Rule rule, std::string keyword) {
    std::string regex = "(" + keyword + ")(?:[\n\r ]+)";
    setRule(rule, regex);
}

void Lexer::loadRules() {
    setRule(Token::Whitespace, "([\t ]+)");
    setRule(Token::Newline, "([\r\n]+)");
    setRule(Token::Comment, "(#[^\n\r]+)");

    setKeywordRule(Token::LetKeyword, "let");
    setKeywordRule(Token::DefKeyword, "def");
    setKeywordRule(Token::TypeKeyword, "type");
    setKeywordRule(Token::AsKeyword, "as");
    setKeywordRule(Token::EndKeyword, "end");
    setKeywordRule(Token::WhileKeyword, "while");
    setKeywordRule(Token::ForKeyword, "for");
    setKeywordRule(Token::InKeyword, "in");
    setKeywordRule(Token::IfKeyword, "if");
    setKeywordRule(Token::ElseKeyword, "else");
    setKeywordRule(Token::AndKeyword, "and");
    setKeywordRule(Token::OrKeyword, "or");
    setKeywordRule(Token::NotKeyword, "not");
    setKeywordRule(Token::ContinueKeyword, "continue");
    setKeywordRule(Token::BreakKeyword, "break");
    setKeywordRule(Token::TryKeyword, "try");
    setKeywordRule(Token::ExceptKeyword, "except");
    setKeywordRule(Token::RaiseKeyword, "raise");
    setKeywordRule(Token::FinallyKeyword, "finally");
    setKeywordRule(Token::FromKeyword, "from");
    setKeywordRule(Token::ImportKeyword, "import");
    setKeywordRule(Token::ReturnKeyword, "return");
    setKeywordRule(Token::WithKeyword, "with");
    setKeywordRule(Token::YieldKeyword, "yield");
    setKeywordRule(Token::AsyncKeyword, "async");
    setKeywordRule(Token::DoKeyword, "do");
    setKeywordRule(Token::UnlessKeyword, "unless");
    setKeywordRule(Token::MutableKeyword, "mutable");
    setKeywordRule(Token::SpawnKeyword, "spawn");
    setKeywordRule(Token::CCallKeyword, "ccall");
    setKeywordRule(Token::UsingKeyword, "using");
    setKeywordRule(Token::SizeofKeyword, "sizeof");
    setKeywordRule(Token::StrideofKeyword, "strideof");
    setKeywordRule(Token::NewKeyword, "new");

    setRule(Token::BooleanLiteral, "(true|false)");
    setRule(Token::StringLiteral, "((['\"])((?:.(?!(?<![\\\\])\\2))*.?)\\2)");
    setRule(Token::ImaginaryLiteral, "([0-9]+(\\.[0-9]+)?[ij])");
    setRule(Token::FloatLiteral, "([0-9]+\\.[0-9]+)");
    setRule(Token::IntegerLiteral, "([0-9]+)");

    setRule(Token::OpenBracket, "(\\[)");
    setRule(Token::CloseBracket, "(\\])");
    setRule(Token::OpenParenthesis, "(\\()");
    setRule(Token::CloseParenthesis, "(\\))");
    setRule(Token::OpenBrace, "(\\{)");
    setRule(Token::CloseBrace, "(\\})");
    setRule(Token::Comma, "(,)");
    setRule(Token::Dot, "(\\.)");
    setRule(Token::Colon, "(:)");
    setRule(Token::Semicolon, "(;)");

    setRule(Token::Assignment, "(=[^=])");
    setRule(Token::Operator, "(<-|->|\\+=|!=|==|[\\^\\+\\*\\-[:Sm:]])");

    std::string nameInitialRegex = "[:L*:][:Nl:][:Sc:][:So:]√_";
    std::string nameAfterRegex = nameInitialRegex + "![:N*:][:M*:][:Sk:][:Pc:]";
    setRule(Token::Identifier, "([" + nameInitialRegex + "][" + nameAfterRegex + "]*)");
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
            throw errors::SyntaxError(filename, currentLineNumber, currentColumn, currentLine, token, "code");
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
