set(LIB_FILES ast/visitor.cpp
              ast/nodes.cpp
              codegen.cpp
              compiler.cpp
              diagnostics.cpp
              parser/keywords.cpp
              parser/lexer.cpp
              parser/parser.cpp
              parser/token.cpp
              prettyprinter.cpp
              symboltable.cpp
              typesystem/checker.cpp
              typesystem/types.cpp
              typesystem/visitor.cpp
              typesystem/inferrer.cpp)

add_library(acorn ${LIB_FILES})
