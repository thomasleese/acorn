//
// Created by Thomas Leese on 07/05/2016.
//

#ifndef ACORN_COMPILER_PASS_H
#define ACORN_COMPILER_PASS_H

#include <vector>

namespace acorn {

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

#endif //ACORN_COMPILER_PASS_H
