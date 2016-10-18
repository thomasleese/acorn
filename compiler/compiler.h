//
// Created by Thomas Leese on 14/03/2016.
//

#pragma once

#include <string>
#include <vector>

namespace acorn {

    namespace diagnostics {
        class Diagnostics;
    }

    namespace compiler {

        class Pass;

        class Compiler {

        public:
            Compiler();
            ~Compiler();

            void debug(std::string line) const;
            bool compile(std::string filename);

        private:
            Diagnostics *m_diagnostics;

        };
    }

}
