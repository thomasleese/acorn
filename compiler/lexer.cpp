//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include <unicode/unistr.h>
#include <boost/regex/icu.hpp>

#include "lexer.h"

using namespace acorn;
using namespace acorn::diagnostics;

Lexer::Lexer(std::string filename)
{
    std::ifstream stream;
    stream.open(filename.c_str());

    std::stringstream ss;
    ss << stream.rdbuf();

    m_data = ss.str();
    m_pos = 0;

    m_current_line_number = 0;
    next_line();

    m_indentation.push_back(0);

    Token token;
    update_indentation(token);
}

bool Lexer::next_token(Token &token) {
    if (m_token_buffer.size()) {
        token = m_token_buffer.front();
        m_token_buffer.pop_front();
        return true;
    }

    if (m_pos >= static_cast<int>(m_data.size())) {
        token = make_token();
        token.kind = Token::EndOfFile;
        return true;
    }

    while (true) {
        skip_whitespace();
        skip_comment();

        // read line continuations
        int ch = get();
        if (ch == '\\') {
            skip_whitespace();
            skip_comment();

            int ch2 = get();
            if (ch2 != '\n') {
                return false;
            }

            next_line();
        } else {
            unget();
            break;
        }
    }

    // update line and column details
    token = make_token();

    if (read_name(token)) {
        // do nothing :)
    } else if (read_number(token)) {
        // do nothing :)
    } else if (read_string(token)) {
        // do nothing :)
    } else if (read_operator(token)) {
        // do nothing :)
    } else if (read_delimiter(token)) {
        // do nothing :)
    } else {
        report(SyntaxError(token, "code"));
        return false;
    }

    return true;
}

void Lexer::debug() {
    std::cout << "DEBUG TOKENS" << std::endl;

    Token token;
    while (next_token(token) && token.kind != Token::EndOfFile) {
        std::cout << Token::as_string(token.kind) << " " << token.line_number << ":" << token.column << std::endl;
    }

    std::cout << "END DEBUG TOKENS" << std::endl;
}

int Lexer::get() {
    int c = m_data[m_pos];
    m_pos++;
    return c;
}

void Lexer::unget() {
    m_pos--;
}

Token Lexer::make_token(Token::Kind kind) const {
    Token token;
    token.kind = kind;
    token.filename = m_filename;
    token.line = m_current_line;
    token.line_number = m_current_line_number;
    token.column = m_current_column;
    return token;
}

unsigned int Lexer::skip_whitespace() {
    unsigned int count = 1;

    int c = get();
    while (c == ' ' || c == '\t' || c == '\f') {
        c = get();
        count++;
    }

    unget();
    count--;

    m_current_column += count;

    return count;
}

void Lexer::skip_comment() {
    int c = get();
    if (c != '#') {
        // obviously not a comment
        unget();
        return;
    }

    // read until the end of the line
    while (c != '\n') {
        c = get();

        // no more comment as there is no more anything
        if (c == EOF) {
            return;
        }
    }

    // return \n character to stream
    unget();
}

void Lexer::next_line() {
    m_current_line_number++;
    m_current_column = 1;
    update_current_line();
}

void Lexer::update_current_line() {
    std::getline(std::istringstream(m_data.substr(m_pos, m_data.size())), m_current_line);
}

void Lexer::update_indentation(Token &token) {
    int level = skip_whitespace();
    if (m_indentation.back() == level) {
        // do nothing
    } else if (level > m_indentation.back()) {
        m_indentation.push_back(level);
        m_token_buffer.push_back(make_token(Token::Indent));
    } else {
        auto it = std::find(m_indentation.begin(), m_indentation.end(), level);
        if (it == m_indentation.end()) {
            report(SyntaxError(token, "indentation"));
        } else {
            while (m_indentation.back() > level) {
                m_indentation.pop_back();
                m_token_buffer.push_back(make_token(Token::Deindent));
            }
        }
    }
}

bool Lexer::read_name(Token &token) {
    std::string buffer = m_data.substr(m_pos, m_data.size());

    boost::smatch matcher;
    boost::u32regex pattern = boost::make_u32regex("^([_[:L*:]][_[:L*:][:N*:]]*)");

    if (boost::u32regex_search(buffer, matcher, pattern)) {
        std::string value = matcher.str(1);
        if (buffer.substr(0, value.size()) == value) {
            token.lexeme = value;
            m_current_column += icu::UnicodeString::fromUTF8(value).countChar32();
            m_pos += token.lexeme.size();

            if (!read_keyword(token)) {
                token.kind = Token::Name;
            }

            return true;
        }
    }

    return false;
}

bool Lexer::read_keyword(Token &token) const {
    if (token.lexeme == "let") {
        token.kind = Token::LetKeyword;
    } else if (token.lexeme == "def") {
        token.kind = Token::DefKeyword;
    } else if (token.lexeme == "return") {
        token.kind = Token::ReturnKeyword;
    } else if (token.lexeme == "if") {
        token.kind = Token::IfKeyword;
    } else if (token.lexeme == "else") {
        token.kind = Token::ElseKeyword;
    } else if (token.lexeme == "as") {
        token.kind = Token::AsKeyword;
    } else if (token.lexeme == "end") {
        token.kind = Token::EndKeyword;
    } else if (token.lexeme == "ccall") {
        token.kind = Token::CCallKeyword;
    } else if (token.lexeme == "using") {
        token.kind = Token::UsingKeyword;
    } else if (token.lexeme == "import") {
        token.kind = Token::ImportKeyword;
    } else if (token.lexeme == "type") {
        token.kind = Token::TypeKeyword;
    } else if (token.lexeme == "module") {
        token.kind = Token::ModuleKeyword;
    } else if (token.lexeme == "builtin") {
        token.kind = Token::BuiltinKeyword;
    } else {
        return false;
    }

    return true;
}

bool Lexer::read_number(Token &token) {
    int ch = get();

    if (!isdigit(ch)) {
        unget();
        return false;
    }

    token.kind = Token::Int;

    while (isdigit(ch) || ch == '.') {
        token.lexeme.append(1, ch);
        ch = get();
        m_current_column++;

        if (ch == '.') {
            token.kind = Token::Float;
        }
    }

    unget();

    return true;
}

bool is_quote_char(int ch) {
    return ch == '"';
}

bool Lexer::read_string(Token &token) {
    int ch = get();

    if (!is_quote_char(ch)) {
        unget();
        return false;
    }

    token.kind = Token::String;

    ch = get();
    while (!is_quote_char(ch) && ch != EOF) {
        token.lexeme.append(1, ch);
        ch = get();
        m_current_column++;
    }

    return true;
}

bool Lexer::read_delimiter(Token &token) {
    int ch = get();

    // the following two lines are reversed in the default case below
    token.lexeme.append(1, ch);
    m_current_column++;

    switch (ch) {
        case EOF:
            token.kind = Token::EndOfFile;
            return true;

        case '\n':
            token.kind = Token::Newline;
            next_line();
            update_indentation(token);
            return true;

        case '[':
            token.kind = Token::OpenBracket;
            return true;

        case ']':
            token.kind = Token::CloseBracket;
            return true;

        case '(':
            token.kind = Token::OpenParenthesis;
            return true;

        case ')':
            token.kind = Token::CloseParenthesis;
            return true;

        case '{':
            token.kind = Token::OpenBrace;
            return true;

        case '}':
            token.kind = Token::CloseBrace;
            return true;

        case ',':
            token.kind = Token::Comma;
            return true;

        case '.':
            token.kind = Token::Dot;
            return true;

        case ':':
            token.kind = Token::Colon;
            return true;

        case ';':
            token.kind = Token::Semicolon;
            return true;

        case '=':
            token.kind = Token::Assignment;
            return true;

        default:
            unget();
            token.lexeme.clear();
            m_current_column--;
            return false;
    }
}

bool is_two_char_operator(char c1, char c2) {
    if (c1 == '=' && c2 == '=') {
        return true;
    } else {
        return false;
    }
}

bool Lexer::read_operator(Token &token) {
    int ch = get();
    int ch2 = get();

    if (is_two_char_operator(ch, ch2)) {
        token.kind = Token::Operator;
        token.lexeme.append(1, ch);
        token.lexeme.append(1, ch2);
        m_current_column += 2;
        return true;
    } else {
        unget();
    }

    // TODO check unicode class

    switch (ch) {
        case '>':
        case '<':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':
            token.kind = Token::Operator;
            token.lexeme.append(1, ch);
            m_current_column++;
            return true;

        default:
            break;
    }

    unget();
    return false;
}
