//
// Created by Thomas Leese on 07/05/2016.
//

#ifndef JET_COMPILER_PASS_H
#define JET_COMPILER_PASS_H

#include <vector>

namespace jet {

    namespace errors {
        class CompilerError;
    }

    namespace compiler {

        class Pass {

        public:
            bool has_errors() const;
            void push_error(errors::CompilerError *error);
            errors::CompilerError *next_error();

        private:
            std::vector<errors::CompilerError *> m_errors;

        };

    }

}

#endif //JET_COMPILER_PASS_H
