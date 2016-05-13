//
// Created by Thomas Leese on 13/03/2016.
//

#ifndef ACORN_LEXER_H
#define ACORN_LEXER_H

#include <map>
#include <string>
#include <vector>

#include "compiler/pass.h"
#include "token.h"

namespace acorn {

    class Lexer : public compiler::Pass {
    public:
        explicit Lexer();

        ~Lexer();

    private:
        void setRule(Token::Rule rule, std::string regex);
        void setKeywordRule(Token::Rule rule, std::string keyword);
        void loadRules();

    public:
        std::vector<Token *> tokenise(std::string filename);

    private:
        std::map<Token::Rule, std::string> m_rules;
    };

}

#endif // ACORN_LEXER_H
