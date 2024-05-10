#include <benchmark/benchmark.h>
#include <fstream>

#include "../../src/core/board/board.h"
#include "../../src/core/moves/movegen.h"
#include "test_boards.h"

static void BenchmarkSimpleOnTestBoard(benchmark::State& state, const std::string_view& fen) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    q_core::MoveList move_list;
    for (auto _ : state) {
        q_core::GenerateAllMoves(board, move_list);
    }
    benchmark::DoNotOptimize(move_list);
}

#define BENCHMARK_SIMPLE(name, fen) \
    static void BM_SimpleMovegen##name(benchmark::State &state) { BenchmarkSimpleOnTestBoard(state, fen); } \
    BENCHMARK(BM_SimpleMovegen##name);

CALL_FOR_ALL_TEST_BOARDS(BENCHMARK_SIMPLE)
#undef BENCH_SIMPLE

BENCHMARK_MAIN();
