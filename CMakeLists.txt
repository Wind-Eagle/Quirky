cmake_minimum_required(VERSION 3.14)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)

add_library(types src/types/types.h src/types/types_operations.h src/util/random.h)
set_target_properties(types PROPERTIES LINKER_LANGUAGE CXX)

add_library(util src/util/string.h src/util/geometry.h src/util/io.h src/util/bit.h)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(util types)

add_library(core src/core/board/board.cpp src/core/moves/movegen.cpp src/core/board/hash.h)
target_link_libraries(core util)

add_library(api src/api/api.cpp src/api/uci/uci_protocol.cpp src/api/xboard/xboard_protocol.cpp)
target_link_libraries(api util)

add_executable(Quirky src/main.cpp)
target_link_libraries(Quirky core api util)
