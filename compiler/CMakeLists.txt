find_package(PkgConfig REQUIRED)

pkg_search_module(ICU REQUIRED icu-uc)
pkg_search_module(SPDLOG REQUIRED spdlog)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

find_package(LLVM REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${SPDLOG_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LD_FLAGS} -fexceptions")

include_directories(include)

set(LIBS ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
