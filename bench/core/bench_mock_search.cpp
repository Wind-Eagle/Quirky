#include <benchmark/benchmark.h>

#include <algorithm>
#include <fstream>

#include "../../src/core/board/board.h"
#include "../../src/core/moves/board_manipulation.h"
#include "../../src/core/moves/movegen.h"
#include "../../src/util/random.h"
#include "test_boards.h"

struct FixedSeedRandom8 {
    uint8_t Get() {
        x = (x * y + z);
        return x;
    }
    void SetParams(uint8_t x_val, uint8_t y_val, uint8_t z_val) {
        x = x_val;
        y = y_val;
        z = z_val;
    }

  private:
    uint8_t x;
    uint8_t y;
    uint8_t z;
};

void SortMovesDeterministic(q_core::MoveList& move_list) {
    std::sort(move_list.moves, move_list.moves + move_list.size,
              [&](const q_core::Move& lhs, const q_core::Move& rhs) {
                  auto lhs_value = (lhs.src << 16) + (lhs.dst << 8) + lhs.type;
                  auto rhs_value = (rhs.src << 16) + (rhs.dst << 8) + rhs.type;
                  return lhs_value < rhs_value;
              });
}

void RunMockSearch(q_core::Board& board, uint8_t depth, FixedSeedRandom8 rnd8) {
    static constexpr uint8_t CAPTURE_GEN_PROBABILITY = 76;
    static constexpr uint8_t PROMOTION_GEN_PROBABILITY = 43;
    static constexpr uint8_t TOTAL_PROBABILITY = 128;
    static constexpr uint8_t CAPTURE_MOVES_COUNT = 2;
    static constexpr uint8_t PROMOTION_MOVES_COUNT = 1;
    static constexpr uint8_t SIMPLE_MOVES_COUNT = 19;
    Q_STATIC_ASSERT(CAPTURE_GEN_PROBABILITY + PROMOTION_GEN_PROBABILITY <= TOTAL_PROBABILITY);

    if (depth == 0) {
        return;
    }
    q_core::MoveList move_list;
    auto run_mock_search = [&](uint8_t cnt_of_moves, bool seq) {
        if (move_list.size == 0) {
            return;
        }
        for (uint8_t i = 0; i < cnt_of_moves; i++) {
            q_core::MakeMoveInfo info;
            uint8_t index = seq ? i : rnd8.Get() % move_list.size;
            q_core::Move move = move_list.moves[index];
            q_core::MakeMove(board, move, info);
            if (q_core::WasMoveLegal(board, move)) {
                RunMockSearch(board, depth - 1, rnd8);
            }
            q_core::UnmakeMove(board, move, info);
        }
    };

    uint8_t value = rnd8.Get() % TOTAL_PROBABILITY;
    q_core::GenerateAllCaptures(board, move_list);
    SortMovesDeterministic(move_list);
    if (value < CAPTURE_GEN_PROBABILITY) {
        run_mock_search(CAPTURE_MOVES_COUNT, false);
        return;
    }
    run_mock_search(move_list.size, true);
    q_core::GenerateAllPromotions(board, move_list);
    SortMovesDeterministic(move_list);
    if (value < CAPTURE_GEN_PROBABILITY + PROMOTION_GEN_PROBABILITY) {
        run_mock_search(PROMOTION_MOVES_COUNT, false);
        return;
    }
    run_mock_search(move_list.size, true);
    q_core::GenerateAllSimpleMoves(board, move_list);
    SortMovesDeterministic(move_list);
    run_mock_search(SIMPLE_MOVES_COUNT, false);
}

static void BenchmarkMockSearch(benchmark::State& state, const std::string_view& fen) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    FixedSeedRandom8 rnd8;
    for (auto _ : state) {
        rnd8.SetParams(15, 51, 23);
        RunMockSearch(board, 8, rnd8);
    }
}

static void BenchmarkMockSearchNoisy(benchmark::State& state, const std::string_view& fen) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    FixedSeedRandom8 rnd8;
    for (auto _ : state) {
        rnd8.SetParams(q_util::GetRandom64(), q_util::GetRandom64(), q_util::GetRandom64());
        RunMockSearch(board, 3, rnd8);
    }
}

void RunRecurse(q_core::Board& board, uint8_t depth, FixedSeedRandom8 rnd8) {
    if (depth == 0) {
        return;
    }
    q_core::MoveList move_list;
    q_core::GenerateAllMoves(board, move_list);
    SortMovesDeterministic(move_list);
    if (move_list.size == 0) {
        return;
    }
    for (uint8_t i = 0; i < move_list.size; i++) {
        q_core::MakeMoveInfo info;
        uint8_t index = rnd8.Get() % move_list.size;
        q_core::Move move = move_list.moves[index];
        q_core::MakeMove(board, move, info);
        if (q_core::WasMoveLegal(board, move)) {
            RunMockSearch(board, depth - 1, rnd8);
        }
        q_core::UnmakeMove(board, move, info);
    }
}

static void BenchmarkRecurse(benchmark::State& state, const std::string_view& fen) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    FixedSeedRandom8 rnd8;
    for (auto _ : state) {
        rnd8.SetParams(15, 51, 23);
        RunRecurse(board, 3, rnd8);
    }
}

static void BenchmarkNoisyRecurse(benchmark::State& state, const std::string_view& fen) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    FixedSeedRandom8 rnd8;
    for (auto _ : state) {
        rnd8.SetParams(q_util::GetRandom64(), q_util::GetRandom64(), q_util::GetRandom64());
        RunRecurse(board, 3, rnd8);
    }
}

#define BENCHMARK_RECURSE(name, fen)                                                        \
    static void BM_Recurse##name(benchmark::State& state) { BenchmarkRecurse(state, fen); } \
    BENCHMARK(BM_Recurse##name)->Unit(benchmark::kMicrosecond);
CALL_FOR_ALL_TEST_BOARDS(BENCHMARK_RECURSE)
#undef BENCHMARK_RECURSE

#define BENCHMARK_NOISY_RECURSE(name, fen)                       \
    static void BM_NoisyRecurse##name(benchmark::State& state) { \
        BenchmarkNoisyRecurse(state, fen);                       \
    }                                                            \
    BENCHMARK(BM_NoisyRecurse##name)->Unit(benchmark::kMicrosecond);
CALL_FOR_ALL_TEST_BOARDS(BENCHMARK_NOISY_RECURSE)
#undef BENCHMARK_NOISY_RECURSE

static void BM_MockSearch(benchmark::State& state) {
    BenchmarkMockSearch(state,
                        "r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq - 0 11");
}
BENCHMARK(BM_MockSearch)->Unit(benchmark::kMillisecond);
static void BM_MockSearchNoisy(benchmark::State& state) {
    BenchmarkMockSearchNoisy(state,
                             "r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq - 0 11");
}
BENCHMARK(BM_MockSearchNoisy)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
