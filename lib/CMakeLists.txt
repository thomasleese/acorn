add_library(acorn
  ast/visitor.cpp
  ast/nodes.cpp
  codegen/followers.cpp
  codegen/generator.cpp
  codegen/irbuilder.cpp
  codegen/mangler.cpp
  compiler.cpp
  diagnostics.cpp
  parser/keywords.cpp
  parser/scanner.cpp
  parser/parser.cpp
  parser/token.cpp
  prettyprinter.cpp
  symboltable/builder.cpp
  symboltable/namespace.cpp
  symboltable/symbol.cpp
  typesystem/types.cpp
  typesystem/visitor.cpp
  typesystem/checker.cpp
)

target_include_directories(acorn PUBLIC ../include)
target_compile_features(acorn PUBLIC cxx_std_17)
target_compile_options(acorn PUBLIC -Wall -pedantic)
