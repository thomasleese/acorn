//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef QUARK_LEXER_H
#define QUARK_LEXER_H

#include <map>
#include <string>
#include <vector>

#include "Token.h"

namespace LexicalAnalysis {

    class Lexer {
    public:
        explicit Lexer();

        ~Lexer();

    private:
        void setRule(Token::Rule rule, std::string regex);
        void setKeywordRule(Token::Rule rule, std::string keyword);

        void loadRules();

    public:
        std::vector<Token *> tokenise(std::string filename) const;

    private:
        std::map<Token::Rule, std::string> m_rules;
    };

}

#endif //QUARK_LEXER_H
