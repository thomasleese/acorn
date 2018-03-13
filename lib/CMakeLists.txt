add_library(acorn
  ast/visitor.cpp
  ast/nodes.cpp
  codegen/followers.cpp
  codegen/generator.cpp
  codegen/irbuilder.cpp
  codegen/mangler.cpp
  compiler.cpp
  diagnostics.cpp
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

llvm_map_components_to_libnames(LLVM_LIBS
  core codegen support
  aarch64asmparser aarch64codegen aarch64desc aarch64info
  amdgpuasmparser amdgpucodegen amdgpudesc amdgpuinfo
  armasmparser armcodegen armdesc arminfo
  bpfasmparser bpfcodegen bpfdesc bpfinfo
  hexagonasmparser hexagoncodegen hexagondesc hexagoninfo
  lanaiasmparser lanaicodegen lanaidesc lanaiinfo
  msp430codegen msp430desc msp430info
  mipsasmparser mipscodegen mipsdesc mipsinfo
  nvptxcodegen nvptxdesc nvptxinfo
  sparcasmparser sparccodegen sparcdesc sparcinfo
  systemzasmparser systemzcodegen systemzdesc systemzinfo
  powerpcasmparser powerpccodegen powerpcdesc powerpcinfo
  x86asmparser x86codegen x86desc x86info
  xcorecodegen xcoredesc xcoreinfo
)

target_include_directories(acorn
  PUBLIC ../include ${LLVM_INCLUDE_DIRS}
  PRIVATE ${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS}
)

target_link_libraries(acorn
  PUBLIC spdlog
  PRIVATE ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses
)

target_compile_features(acorn PUBLIC cxx_std_17)

target_compile_options(acorn
  PUBLIC -Wall -pedantic ${LLVM_CXX_FLAGS}
)

target_compile_definitions(acorn
  PUBLIC ${LLVM_DEFINITIONS}
)
