set(LIB_FILES codegen.cpp
              compiler.cpp
              diagnostics.cpp
              lexer.cpp
              parser.cpp
              prettyprinter.cpp
              symboltable.cpp
              token.cpp
              typesystem/checker.cpp
              typesystem/types.cpp
              ast/visitor.cpp
              ast/nodes.cpp
              typesystem/visitor.cpp
              typesystem/inferrer.cpp
              parser/keywords.cpp)

add_library(acorn ${LIB_FILES})
