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

llvm_map_components_to_libnames(LLVM_LIBS all)

target_include_directories(acorn
  PUBLIC ../include ${LLVM_INCLUDE_DIRS}
  PRIVATE ${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${SPDLOG_INCLUDE_DIRS}
)

target_link_libraries(acorn
  PUBLIC ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses
)

target_compile_features(acorn PUBLIC cxx_std_17)

target_compile_options(acorn
  PUBLIC -Wall -pedantic ${LLVM_CXX_FLAGS}
)

target_compile_definitions(acorn
  PUBLIC ${LLVM_DEFINITIONS}
)
