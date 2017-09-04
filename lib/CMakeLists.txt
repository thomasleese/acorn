set(LIB_FILES ast/visitor.cpp
              ast/nodes.cpp
              codegen/followers.cpp
              codegen/generator.cpp
              codegen/irbuilder.cpp
              codegen/mangler.cpp
              compiler.cpp
              diagnostics.cpp
              parser/keywords.cpp
              parser/lexer.cpp
              parser/parser.cpp
              parser/token.cpp
              prettyprinter.cpp
              symboltable/builder.cpp
              symboltable/namespace.cpp
              symboltable/symbol.cpp
              typesystem/types.cpp
              typesystem/visitor.cpp
              typesystem/checker.cpp)

add_library(acorn ${LIB_FILES})
