cmake_minimum_required(VERSION 3.14)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2")

add_library(util src/util/io.h src/util/macro.h src/util/error.h src/util/hash.h src/util/random.h src/util/bit.h src/util/string.h)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(util)

add_library(core src/core/board/board.cpp src/core/board/geometry.h src/core/board/types.h src/core/util.h src/core/moves/movegen.cpp src/core/moves/attack.cpp src/core/moves/board_manipulation.cpp src/core/moves/magic.cpp src/core/moves/move.cpp)
target_link_libraries(core util)

add_library(api src/api/api.cpp src/api/uci/uci_protocol.cpp src/api/xboard/xboard_protocol.cpp)
target_link_libraries(api util)

add_executable(Quirky src/main.cpp)
target_link_libraries(Quirky core api util)

find_package(GTest)
if(GTest_FOUND)
    include(GoogleTest)
    add_executable(core_board_test test/core/board/fen_parse_test.cpp)
    target_link_libraries(core_board_test core)
    target_link_libraries(core_board_test GTest::GTest GTest::Main)

    add_executable(core_moves_test test/core/moves/pseudolegal_movegen_test.cpp test/core/moves/util.h test/core/moves/util.cpp test/core/moves/pseudolegal_board_manipulation_test.cpp)
    target_link_libraries(core_moves_test core)
    target_link_libraries(core_moves_test GTest::GTest GTest::Main)
endif()
