//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"

using namespace acorn;
using namespace acorn::diagnostics;

Lexer::Lexer(std::string filename)
{
    m_stream.open(filename.c_str());

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

    if (!m_stream.good()) {
        token = make_token();
        token.kind = Token::EndOfFile;
        return true;
    }

    while (true) {
        skip_whitespace();
        skip_comment();

        // read line continuations
        int ch = m_stream.get();
        if (ch == '\\') {
            skip_whitespace();
            skip_comment();

            int ch2 = m_stream.get();
            if (ch2 != '\n') {
                return false;
            }

            next_line();
        } else {
            m_stream.unget();
            break;
        }
    }

    // update line and column details
    token = make_token();

    if (read_identifier(token)) {
        // do nothing :)
    } else if (read_number(token)) {
        // do nothing :)
    } else if (read_string(token)) {
        // do nothing :)
    } else if (read_delimiter(token)) {
        // do nothing :)
    } else if (read_operator(token)) {
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

    int c = m_stream.get();
    while (c == ' ' || c == '\t' || c == '\f') {
        c = m_stream.get();
        count++;
    }

    m_stream.unget();
    count--;

    m_current_column += count;

    return count;
}

void Lexer::skip_comment() {
    int c = m_stream.get();
    if (c != '#') {
        // obviously not a comment
        m_stream.unget();
        return;
    }

    // read until the end of the line
    while (c != '\n') {
        c = m_stream.get();

        // no more comment as there is no more anything
        if (c == EOF) {
            return;
        }
    }

    // return \n character to stream
    m_stream.unget();
}

void Lexer::next_line() {
    m_current_line_number++;
    m_current_column = 1;
    update_current_line();
}

void Lexer::update_current_line() {
    long long int pos = m_stream.tellg();
    std::getline(m_stream, m_current_line);
    m_stream.seekg(pos);
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

bool Lexer::read_identifier(Token &token) {
    int ch = m_stream.get();

    if (!isalpha(ch)) {
        m_stream.unget();
        return false;
    }

    while (isalpha(ch) || isdigit(ch)) {
        token.lexeme.append(1, ch);
        ch = m_stream.get();
    }

    m_stream.unget();

    m_current_column += token.lexeme.size();

    if (!read_keyword(token)) {
        token.kind = Token::Name;
    }

    return true;
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
    } else {
        return false;
    }

    return true;
}

bool Lexer::read_number(Token &token) {
    int ch = m_stream.get();

    if (!isdigit(ch)) {
        m_stream.unget();
        return false;
    }

    token.kind = Token::IntegerLiteral;

    while (isdigit(ch) || ch == '.') {
        token.lexeme.append(1, ch);
        ch = m_stream.get();
        m_current_column++;

        if (ch == '.') {
            token.kind = Token::FloatLiteral;
        }
    }

    m_stream.unget();

    return true;
}

bool Lexer::read_string(Token &token) {
    int ch = m_stream.get();

    if (ch != '"' && ch != '\'') {
        m_stream.unget();
        return false;
    }

    token.kind = Token::StringLiteral;

    while (ch != '"' && ch != '\'') {
        token.lexeme.append(1, ch);
        ch = m_stream.get();
        m_current_column++;
    }

    m_stream.unget();

    return true;
}

bool Lexer::read_delimiter(Token &token) {
    int ch = m_stream.get();
    int ch2 = m_stream.get();

    if (ch == '-' && ch2 == '>') {
        token.kind = Token::Arrow;
        token.lexeme = "->";
        m_current_column += 2;
        return true;
    } else {
        m_stream.unget();
    }

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

        // delimiters
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

        case '|':
            token.kind = Token::Pipe;
            return true;

        default:
            m_stream.unget();
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
    int ch = m_stream.get();
    int ch2 = m_stream.get();

    if (is_two_char_operator(ch, ch2)) {
        token.kind = Token::Operator;
        token.lexeme.append(1, ch);
        token.lexeme.append(1, ch2);
        m_current_column += 2;
        return true;
    } else {
        m_stream.unget();
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
            token.kind = Token::Operator;
            token.lexeme.append(1, ch);
            m_current_column++;
            return true;

        default:
            break;
    }

    m_stream.unget();
    return false;
}
