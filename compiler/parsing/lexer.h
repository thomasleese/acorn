//
// Created by Thomas Leese on 13/03/2016.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "../pass.h"
#include "token.h"

namespace acorn {

    /**
     * Lexical Analysis. Takes source code input and provides a stream of
     * tokens as output.
     */
    class Lexer : public compiler::Pass {
    public:
        Lexer();

    private:
        void set_rule(Token::Kind kind, std::string regex);
        void set_keyword_rule(Token::Kind kind, std::string keyword);
        void load_rules();

    public:
        /**
         * Tokenise a file containing source code into a list of tokens.
         */
        std::vector<Token> tokenise(std::string filename);

    private:
        std::map<Token::Kind, std::string> m_rules;
    };

}
