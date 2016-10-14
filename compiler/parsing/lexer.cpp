//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include <unicode/unistr.h>
#include <boost/regex/icu.hpp>

#include "../errors.h"

#include "lexer.h"

using namespace acorn;

Lexer::Lexer() {
    load_rules();
}

void Lexer::set_rule(Token::Kind kind, std::string regex) {
    m_rules[kind] = "^" + regex;
}

void Lexer::set_keyword_rule(Token::Kind kind, std::string keyword) {
    std::string regex = "(" + keyword + ")(?:[\n\r ]+)";
    set_rule(rule, regex);
}

void Lexer::load_rules() {
    set_rule(Token::Whitespace, "([\t ]+)");
    set_rule(Token::Newline, "([\r\n]+)");
    set_rule(Token::Comment, "(#[^\r\n]*[\r\n]+)");

    set_keyword_rule(Token::LetKeyword, "let");
    set_keyword_rule(Token::DefKeyword, "def");
    set_keyword_rule(Token::TypeKeyword, "type");
    set_keyword_rule(Token::AsKeyword, "as");
    set_keyword_rule(Token::EndKeyword, "end");
    set_keyword_rule(Token::WhileKeyword, "while");
    set_keyword_rule(Token::ForKeyword, "for");
    set_keyword_rule(Token::InKeyword, "in");
    set_keyword_rule(Token::IfKeyword, "if");
    set_keyword_rule(Token::ElseKeyword, "else");
    set_keyword_rule(Token::ThenKeyword, "then");
    set_keyword_rule(Token::ContinueKeyword, "continue");
    set_keyword_rule(Token::BreakKeyword, "break");
    set_keyword_rule(Token::TryKeyword, "try");
    set_keyword_rule(Token::ExceptKeyword, "except");
    set_keyword_rule(Token::RaiseKeyword, "raise");
    set_keyword_rule(Token::FinallyKeyword, "finally");
    set_keyword_rule(Token::FromKeyword, "from");
    set_keyword_rule(Token::ImportKeyword, "import");
    set_keyword_rule(Token::ReturnKeyword, "return");
    set_keyword_rule(Token::WithKeyword, "with");
    set_keyword_rule(Token::YieldKeyword, "yield");
    set_keyword_rule(Token::AsyncKeyword, "async");
    set_keyword_rule(Token::RepeatKeyword, "repeat");
    set_keyword_rule(Token::UnlessKeyword, "unless");
    set_keyword_rule(Token::MutableKeyword, "mutable");
    set_keyword_rule(Token::SpawnKeyword, "spawn");
    set_keyword_rule(Token::CCallKeyword, "ccall");
    set_keyword_rule(Token::UsingKeyword, "using");
    set_keyword_rule(Token::NewKeyword, "new");
    set_keyword_rule(Token::InoutKeyword, "inout");
    set_keyword_rule(Token::ProtocolKeyword, "protocol");
    set_keyword_rule(Token::EnumKeyword, "enum");
    set_keyword_rule(Token::SwitchKeyword, "switch");
    set_keyword_rule(Token::CaseKeyword, "case");
    set_keyword_rule(Token::DefaultKeyword, "case");

    set_rule(Token::StringLiteral, "((['\"])((?:.(?!(?<![\\\\])\\2))*.?)\\2)");
    set_rule(Token::ImaginaryLiteral, "([0-9]+(\\.[0-9]+)?[ij])");
    set_rule(Token::FloatLiteral, "([0-9]+\\.[0-9]+)");
    set_rule(Token::IntegerLiteral, "([0-9]+)");

    set_rule(Token::OpenBracket, "(\\[)");
    set_rule(Token::CloseBracket, "(\\])");
    set_rule(Token::OpenParenthesis, "(\\()");
    set_rule(Token::CloseParenthesis, "(\\))");
    set_rule(Token::OpenBrace, "(\\{)");
    set_rule(Token::CloseBrace, "(\\})");
    set_rule(Token::Comma, "(,)");
    set_rule(Token::Dot, "(\\.)");
    set_rule(Token::Colon, "(:)");
    set_rule(Token::Semicolon, "(;)");

    set_rule(Token::Assignment, "(=[^=])");
    set_rule(Token::Operator, "(and|or|not|<-|->|>=|<=|\\+=|!=|==|[\\^\\+\\*\\-[:Sm:]])");

    std::string nameInitialRegex = "[:L*:][:Nl:][:Sc:][:So:]√_";
    std::string nameAfterRegex = nameInitialRegex + "![:N*:][:M*:][:Sk:][:Pc:]";
    set_rule(Token::Identifier, "([" + nameInitialRegex + "][" + nameAfterRegex + "]*)");
}

std::string Lexer::read_file(std::string filename) const {
    std::stringstream buffer;

    std::ifstream in(filename.c_str());
    while (in.good()) {
        std::string line;
        std::getline(in, line);

        buffer << line;
        buffer << "\n";
    }

    return buffer.str();
}

std::vector<Token> Lexer::tokenise(std::string filename) {
    std::string buffer = read_file(filename);

    int currentLineNumber = 1;
    int currentColumn = 0;
    std::string currentLine;

    std::getline(std::istringstream(buffer), currentLine);

    unsigned long pos = 0;
    unsigned long end = buffer.length();

    boost::smatch matcher;

    std::vector<Token> tokens;

    while (pos < end) {
        std::string substr = buffer.substr(pos, end - pos);

        bool found = false;

        for (auto it = m_rules.begin(); it != m_rules.end(); it++) {
            Token::Kind kind = it->first;
            boost::u32regex pattern = boost::make_u32regex(it->second);

            if (boost::u32regex_search(substr, matcher, pattern)) {
                std::string value = matcher.str(1);

                if (substr.substr(0, value.size()) == value) {
                    if (rule != Token::Whitespace && rule != Token::Comment) {
                        Token token(kind, value);

                        token.filename = filename;
                        token.line = currentLine;
                        token.column = currentColumn;
                        token.lineNumber = currentLineNumber;

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
            push_error(new errors::SyntaxError(filename,
                                               currentLineNumber,
                                               currentColumn,
                                               currentLine,
                                               token,
                                               "code"));
            return tokens;  // we can't really continue at this point
        }
    }

    Token endOfFileToken(Token::EndOfFile, "");
    endOfFileToken->filename = filename;
    endOfFileToken->line = currentLine;
    endOfFileToken->column = currentColumn;
    endOfFileToken->lineNumber = currentLineNumber;
    tokens.push_back(endOfFileToken);

    return tokens;
}
