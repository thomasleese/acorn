set(LIB_FILES codegen.cpp
              compiler.cpp
              diagnostics.cpp
              parser.cpp
              prettyprinter.cpp
              symboltable.cpp
              typesystem/checker.cpp
              typesystem/types.cpp
              ast/visitor.cpp
              ast/nodes.cpp
              typesystem/visitor.cpp
              typesystem/inferrer.cpp
              parser/keywords.cpp
              parser/lexer.cpp
              parser/token.cpp)

add_library(acorn ${LIB_FILES})
