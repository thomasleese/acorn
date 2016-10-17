//
// Created by Thomas Leese on 13/03/2016.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include "../errors.h"

#include "lexer.h"

using namespace acorn;

Lexer::Lexer(std::string filename) {
    m_stream.open(filename.c_str());

    m_current_column = 0;
    m_current_line_number = 1;
    std::getline(m_stream, m_current_line);

    m_indentation.push_back(0);
    update_indentation();
}

Token Lexer::next_token() {
    if (m_token_buffer.size()) {
        auto token = m_token_buffer.front();
        m_token_buffer.pop_front();
        return token;
    }

    Token token = make_token();

    if (!m_stream.good()) {
        token.kind = Token::EndOfFile;
        return token;
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
                throw false;
            }

            next_line();
        } else {
            m_stream.unget();
            break;
        }
    }

    if (read_identifier(token)) {
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
        throw false;
    }

    return token;
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
    unsigned int count = 0;

    int c = ' ';
    while (c == ' ' || c == '\t' || c == '\f' || c == EOF) {
        c = m_stream.get();
        count++;
    }

    m_stream.unget();
    count--;

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
    m_current_column = 0;
    std::getline(m_stream, m_current_line);
}

void Lexer::update_indentation() {
    unsigned int level = skip_whitespace();
    if (m_indentation.back() == level) {
        // do nothing
    } else if (level > m_indentation.back()) {
        m_indentation.push_back(level);
        m_token_buffer.push_back(make_token(Token::Indent));
    } else {
        auto it = std::find(m_indentation.begin(), m_indentation.end(), level);
        if (it == m_indentation.end()) {
            throw false;
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
        m_current_column++;
    }

    m_stream.unget();

    if (!read_keyword(token)) {
        token.kind = Token::Name;
    }

    return true;
}

bool Lexer::read_keyword(Token &token) const {
    if (token.lexeme == "let") {
        token.kind = Token::LetKeyword;
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

bool Lexer::read_operator(Token &token) {
    int ch = m_stream.get();
    int ch2 = m_stream.get();

    // TODO check unicode class
    if (ch == '=' && ch2 == '=') {
        token.kind = Token::Operator;
        token.lexeme.append(1, ch);
        token.lexeme.append(1, ch2);
        m_current_column += 2;
    }

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

bool Lexer::read_delimiter(Token &token) {
    int ch = m_stream.get();

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
            update_indentation();
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

        default:
            m_stream.unget();
            token.lexeme.clear();
            m_current_column--;
            return false;
    }
}
