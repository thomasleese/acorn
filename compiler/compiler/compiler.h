//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef ACORN_COMPILER_COMPILER_H
#define ACORN_COMPILER_COMPILER_H

#include <string>
#include <vector>

namespace acorn {

    namespace compiler {

        class Pass;

        class Compiler {

        public:
            Compiler();
            ~Compiler();

            bool check_pass(Pass *pass) const;

            void debug(std::string line);
            bool compile(std::string filename);

        };
    }

}

#endif // ACORN_COMPILER_COMPILER_H
