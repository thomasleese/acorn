//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef JET_COMPILER_H
#define JET_COMPILER_H

#include <string>

namespace jet {

    class Compiler {

    public:
        Compiler();
        ~Compiler();

        void debug(std::string line);
        void compile(std::string filename);

    };

}

#endif // JET_COMPILER_H
