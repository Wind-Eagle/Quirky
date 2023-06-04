cmake_minimum_required(VERSION 3.14)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)

add_library(util src/util/io.h)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(util)

add_library(core src/core/board/board.h src/core/board/board.cpp src/core/board/geometry.h src/core/board/hash.h src/core/board/types.h)
target_link_libraries(core util)

add_library(api src/api/api.cpp src/api/uci/uci_protocol.cpp src/api/xboard/xboard_protocol.cpp src/util/macro.h src/util/error.h)
target_link_libraries(api util)

add_executable(Quirky src/main.cpp)
target_link_libraries(Quirky core api util)