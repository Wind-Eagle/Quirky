cmake_minimum_required(VERSION 3.14)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -mbmi2")

add_library(util src/util/io.h src/util/macro.h src/util/error.h src/util/hash.h src/util/random.h src/util/bit.h src/util/string.h src/util/math.h)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(util)

add_library(core src/core/board/board.cpp src/core/board/geometry.h src/core/board/types.h src/core/util.h src/core/moves/movegen.cpp src/core/moves/attack.cpp src/core/moves/board_manipulation.cpp src/core/moves/magic.cpp src/core/moves/move.cpp)
target_link_libraries(core util)

add_library(eval src/eval/feature.h src/eval/evaluator.cpp src/eval/model.h src/eval/pawns.h src/eval/score.h src/eval/eval_features.cpp src/eval/psq.h)
target_link_libraries(eval core util)

add_library(search src/search/control/control.cpp src/search/control/stat.cpp src/search/control/time.cpp src/search/position/move_picker.cpp src/search/position/position.cpp src/search/position/repetition_table.cpp src/search/position/transposition_table.cpp src/search/searcher/launcher.cpp src/search/searcher/searcher.cpp)
target_link_libraries(search core eval util)

add_library(api src/api/api.cpp src/api/uci/protocol.cpp src/api/uci/parser.cpp src/api/uci/logger.cpp src/api/uci/interactor.cpp)
target_link_libraries(api core eval search util)

add_executable(Quirky src/main.cpp)
target_link_libraries(Quirky core api util eval search)

add_executable(eval_model_sampler
    tools/eval/model/sampler/main.cpp
    tools/eval/model/sampler/reader.cpp
    tools/eval/model/sampler/calcer.cpp
    tools/eval/model/sampler/weighter.cpp
    tools/eval/model/sampler/writer.cpp
)
target_link_libraries(eval_model_sampler eval)

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
