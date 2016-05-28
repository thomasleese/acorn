//
// Created by Thomas Leese on 28/05/2016.
//

#ifndef ACORN_CODEGEN_UNIT_H
#define ACORN_CODEGEN_UNIT_H

#include <map>
#include "../ast/nodes.h"

namespace llvm {
    class Value;
    class Function;
}

namespace acorn {

    namespace ast {
        struct FunctionDefinition;
    }

    namespace types {
        class Method;
    }

    namespace codegen {

        class Unit {
        public:
            enum Kind { Value, Function };

            Unit(llvm::Value *value);
            Unit(std::string name, std::map<types::Method *, ast::FunctionDefinition *> definitions);

            llvm::Value *llvm_value();

            std::string function_name();
            ast::FunctionDefinition *function_definition(types::Method *method);

        private:
            Kind m_kind;

            llvm::Value *m_llvm_value;
            std::string m_function_name;
            std::map<types::Method *, ast::FunctionDefinition *> m_function_definitions;
        };

    }

}

#endif //ACORN_UNIT_H
