cmake_minimum_required(VERSION 3.14)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O3 -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt -mavx -mavx2 -mbmi -mbmi2")

add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wnon-virtual-dtor -Woverloaded-virtual
    -Wold-style-cast -Wcast-qual -Wsign-promo
    -Wzero-as-null-pointer-constant -Wextra-semi
    -Werror
)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
    message(STATUS "LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported: <${error}>")
endif()

add_library(util src/util/io.h src/util/macro.h src/util/error.h src/util/hash.h src/util/random.h src/util/bit.h src/util/string.h src/util/math.h src/util/channel.h src/util/processor.h)

set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(util)

add_library(core src/core/board/board.cpp src/core/board/geometry.h src/core/board/types.h src/core/util.h src/core/moves/movegen.cpp src/core/moves/attack.cpp src/core/moves/board_manipulation.cpp src/core/moves/magic.cpp src/core/moves/move.cpp)
target_link_libraries(core util)

add_library(eval src/eval/evaluator.cpp src/eval/model.h src/eval/pawns.cpp src/eval/score.h src/eval/eval_features.cpp src/eval/psq.h)
target_link_libraries(eval core util)

add_library(search src/search/control/control.cpp src/search/control/stat.cpp src/search/control/time.cpp src/search/position/move_picker.cpp src/search/position/position.cpp src/search/position/repetition_table.cpp src/search/position/transposition_table.cpp src/search/searcher/launcher.cpp src/search/searcher/searcher.cpp)
target_link_libraries(search core eval util)

add_library(api src/api/api.cpp src/api/uci/protocol.cpp src/api/uci/parser.cpp src/api/uci/logger.cpp src/api/uci/interactor.cpp)
target_link_libraries(api core eval search util)

add_executable(Quirky src/main.cpp)
target_link_libraries(Quirky core api util eval search)

add_executable(eval_sampler
    tools/eval/sampler/main.cpp
    tools/eval/sampler/reader.cpp
    tools/eval/sampler/calcer.cpp
    tools/eval/sampler/weighter.cpp
    tools/eval/sampler/writer.cpp
)
target_link_libraries(eval_sampler eval)

add_executable(eval_tuner
    tools/eval/tuner/main.cpp
    tools/eval/tuner/reader.cpp
    tools/eval/tuner/dataset.cpp
    tools/eval/tuner/calcer.cpp
    tools/eval/tuner/learner.cpp
)
target_link_libraries(eval_tuner search)
target_compile_definitions(eval_tuner PUBLIC CHANGE_EVAL_CONSTS)

find_package(GTest)
if(GTest_FOUND)
    include(GoogleTest)
    add_executable(test_core test/core/fen_parse_test.cpp test/core/simple_movegen_test.cpp test/core/util.h test/core/util.cpp test/core/board_manipulation_test.cpp)
    target_link_libraries(test_core core)
    target_link_libraries(test_core GTest::GTest GTest::Main)

    add_executable(test_eval test/eval/evaluator_test.cpp)
    target_link_libraries(test_eval eval)
    target_link_libraries(test_eval GTest::GTest GTest::Main)
endif()

find_package(benchmark)
if(benchmark_FOUND)
    add_executable(bench_core_movegen bench/core/bench_movegen.cpp)
    target_link_libraries(bench_core_movegen core)
    target_link_libraries(bench_core_movegen benchmark::benchmark)

    add_executable(bench_core_board_manipulation bench/core/bench_board_manipulation.cpp)
    target_link_libraries(bench_core_board_manipulation core)
    target_link_libraries(bench_core_board_manipulation benchmark::benchmark)

    add_executable(bench_core_mock_search bench/core/bench_mock_search.cpp)
    target_link_libraries(bench_core_mock_search core)
    target_link_libraries(bench_core_mock_search benchmark::benchmark)
endif()
