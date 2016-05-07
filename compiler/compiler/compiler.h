//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef JET_COMPILER_COMPILER_H
#define JET_COMPILER_COMPILER_H

#include <string>
#include <vector>

namespace jet {

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

#endif // JET_COMPILER_COMPILER_H
