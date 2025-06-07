cmake_minimum_required(VERSION 3.18)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O2 -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt -march=native")

add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-qual -Wsign-promo
    -Wzero-as-null-pointer-constant -Wextra-semi
    -Werror
)

option (NO_AVX2 OFF)
if(NO_AVX2)
    MESSAGE(STATUS "BMI2 AND AVX2 is off")
    add_definitions(-DNO_AVX2=1)
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mbmi -mbmi2")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
    message(STATUS "LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported: <${error}>")
endif()

include_directories("${PROJECT_BINARY_DIR}")
include_directories(src)

add_custom_command(
    COMMAND ${PROJECT_SOURCE_DIR}/src/incbin/embed.py --input ${PROJECT_SOURCE_DIR}/src/incbin/model.qnne --output ${PROJECT_BINARY_DIR}/model_weights.h
    DEPENDS ${PROJECT_SOURCE_DIR}/src/incbin/embed.py  ${PROJECT_SOURCE_DIR}/src/incbin/model.qnne
    OUTPUT ${PROJECT_BINARY_DIR}/model_weights.h
)

add_library(util INTERFACE src/util/io.h src/util/macro.h src/util/error.h src/util/hash.h src/util/random.h src/util/bit.h src/util/string.h src/util/math.h)

set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(util)

add_library(core src/core/board/board.cpp src/core/board/geometry.cpp src/core/board/types.h src/core/util.h src/core/moves/movegen.cpp src/core/moves/attack.cpp src/core/moves/board_manipulation.cpp src/core/moves/magic.cpp src/core/moves/move.cpp)
target_link_libraries(core util)

add_library(eval src/eval/evaluator.cpp src/eval/model.cpp src/eval/score.h src/eval/layers.h ${PROJECT_BINARY_DIR}/model_weights.h)
target_link_libraries(eval core util)

add_library(search src/search/control/control.cpp src/search/control/stat.cpp src/search/control/time.cpp src/search/position/move_picker.cpp src/search/position/position.cpp src/search/position/repetition_table.cpp src/search/position/transposition_table.cpp src/search/searcher/launcher.cpp src/search/searcher/searcher.cpp)
target_link_libraries(search core eval util)

add_library(api src/api/api.cpp src/api/uci/protocol.cpp src/api/uci/parser.cpp src/api/uci/logger.cpp src/api/uci/interactor.cpp)
target_link_libraries(api core eval search util)

add_executable(Quirky src/main.cpp)
target_link_libraries(Quirky core api util eval search)

add_executable(model_sampler
    tools/model_sampler/main.cpp
    tools/model_sampler/reader.cpp
    tools/model_sampler/writer.cpp
)
target_link_libraries(model_sampler core eval util)
