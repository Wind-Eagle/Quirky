#include "launcher.h"

#include <utility>

#include "util/string.h"
#include "core/board/board.h"
#include "core/moves/move.h"
#include "core/moves/movegen.h"
#include "eval/score.h"
#include "search/control/control.h"
#include "search/control/stat.h"
#include "search/position/transposition_table.h"
#include "searcher.h"
#include "util/bit.h"
#include "util/io.h"

namespace q_search {

uint8_t GetRTByteSizeLog(size_t moves_count) {
    return q_util::GetHighestBit((moves_count + Searcher::MAX_DEPTH) * 4) + 3;
}

void ProcessPositionMoves(Position& position, const std::vector<q_core::Move>& moves,
                          RepetitionTable& rt) {
    RepetitionTable helper_rt{GetRTByteSizeLog(moves.size())};
    for (const auto& move : moves) {
        if (helper_rt.Has(position.board.hash)) {
            rt.Insert(position.board.hash);
        } else {
            helper_rt.Insert(position.board.hash);
        }
        q_core::MakeMoveInfo make_move_info;
        q_eval::Evaluator::EvaluatorUpdateInfo evaluator_update_info;
        position.MakeMove(move, make_move_info, evaluator_update_info);
    }
}

void SearchLauncher::Start(const Position& start_position, const std::vector<q_core::Move>& moves,
                           time_control_t time_control, depth_t max_depth) {
    Join();
    control_.Reset();
    tt_.NextPosition();
    thread_ = std::thread([this, start_position, moves, time_control, max_depth]() {
        StartMainThread(start_position, moves, time_control, max_depth);
    });
}

uint64_t GetNPS(const SearchStat& stat, time_t time_since_start) {
    return time_since_start == 0 ? stat.GetNodesCount()
                                 : stat.GetNodesCount() * 1000 / time_since_start;
}

void PrintSearchResult(const SearchResult& result, const SearchStat& stat,
                       time_t time_since_start) {
    std::vector<std::string> moves;
    if (!IsMoveNull(result.best_move)) {
        moves.push_back(q_core::CastMoveToString(result.best_move));
    }
    for (const auto& move : result.pv) {
        moves.push_back(q_core::CastMoveToString(move));
    }
    std::string pv_str = q_util::ConcatenateStrings(moves.begin(), moves.end());
    std::string score_str;
    if (result.bound_type != Exact || !q_eval::IsScoreMate(result.score)) {
        score_str =
            "score cp " + std::to_string(result.score) +
            (result.bound_type == Lower ? " lowerbound"
                                        : (result.bound_type == Upper ? " upperbound" : ""));
    } else {
        int num_of_moves_to_mate = (std::abs(q_eval::SCORE_MATE) - std::abs(result.score)) / 2;
        if (result.score < 0) {
            num_of_moves_to_mate *= -1;
        }
        score_str = "score mate " + std::to_string(num_of_moves_to_mate);
    }
    q_util::Print("info depth", static_cast<int>(result.depth), "time", time_since_start, score_str,
                  "nodes", stat.GetNodesCount(), "nps", GetNPS(stat, time_since_start),
                  !pv_str.empty() ? "pv " + pv_str : "");
}

void PrintRootMove(const RootMove& root_move) {
    q_util::Print("info depth", root_move.depth, "currmove",
                  q_core::CastMoveToString(root_move.move), "currmovenumber", root_move.number + 1);
}

void PrintNodes(const SearchStat& stat, time_t time_since_start) {
    q_util::Print("info nodes", stat.GetNodesCount());
    q_util::Print("info nps", GetNPS(stat, time_since_start));
}

void PrintBestMove(const q_core::Move move) {
    q_util::Print("bestmove", q_core::CastMoveToString(move));
}

q_core::Move GetRandomMove(Position& position, bool& has_two_legal_moves) {
    q_core::MoveList move_list;
    q_core::Movegen movegen(position.board);
    movegen.GenerateAllMoves(position.board, move_list);
    q_core::Move random_move = q_core::NULL_MOVE;
    for (size_t i = 0; i < move_list.size; i++) {
        q_core::MakeMoveInfo make_move_info;
        q_eval::Evaluator::EvaluatorUpdateInfo evaluator_update_info;
        bool legal = position.MakeMove(move_list.moves[i], make_move_info, evaluator_update_info);
        if (!legal) {
            continue;
        }
        position.UnmakeMove(move_list.moves[i], make_move_info, evaluator_update_info);
        if (!q_core::IsMoveNull(random_move)) {
            has_two_legal_moves = true;
            return random_move;
        }
        random_move = move_list.moves[i];
    }
    has_two_legal_moves = false;
    return random_move;
}

SearchLauncher::~SearchLauncher() { Join(); }

void SearchLauncher::StartMainThread(const Position& start_position,
                                     const std::vector<q_core::Move>& moves,
                                     time_control_t time_control, depth_t max_depth) {
    RepetitionTable rt{GetRTByteSizeLog(moves.size())};
    Position position = start_position;
    ProcessPositionMoves(position, moves, rt);

    bool has_two_legal_moves = false;
    const q_core::Move random_move = GetRandomMove(position, has_two_legal_moves);
    if (q_core::IsMoveNull(random_move)) {
        q_util::PrintError("This is a position with no legal moves: either mate or stalemate");
        return;
    }
    if (!has_two_legal_moves && std::holds_alternative<GameTimeControl>(time_control)) {
        PrintBestMove(random_move);
        return;
    }

    SearchStat stat;
    Searcher searcher(tt_, rt, position, control_, stat);
    SearchTimer timer(time_control, position, stat);
    std::thread search_thread = std::thread([&]() { searcher.Run(max_depth); });

    SearchResult final_result{};
    final_result.depth = 0;

    static constexpr time_t NODES_UPDATE_TICK = 3000;
    time_t nodes_update_timer = 0;
    for (;;) {
        auto time_left = timer.GetWaitTime();
        auto event = control_.Wait(time_left);
        if (event == SearchControl::Event::Stop) {
            break;
        }
        time_t time_since_start = timer.GetTimeSinceStart();

        if (event == SearchControl::Event::NewResult) {
            std::vector<SearchResult> results = control_.GetResults();
            for (auto& result : results) {
                if (result.bound_type == Exact && result.depth >= final_result.depth) {
                    PrintSearchResult(result, stat, time_since_start);
                    final_result = std::move(result);
                    timer.ProcessNextDepth(result);
                } else if (result.bound_type == Lower && result.depth >= final_result.depth) {
                    if (control_.AreDetailedResultsEnabled()) {
                        PrintSearchResult(result, stat, time_since_start);
                    }
                    final_result = std::move(result);
                } else if (result.bound_type == Upper && result.depth >= final_result.depth) {
                    if (control_.AreDetailedResultsEnabled()) {
                        PrintSearchResult(result, stat, time_since_start);
                    }
                }
            }
            if (final_result.depth >= max_depth) {
                control_.Stop();
            }
        }

        if (event == SearchControl::Event::RootMove) {
            std::vector<RootMove> root_moves = control_.GetRootMoves();
            for (auto& root_move : root_moves) {
                PrintRootMove(root_move);
            }
        }

        if (time_left == std::chrono::milliseconds(0)) {
            control_.Stop();
        }
        if (time_since_start >= nodes_update_timer + NODES_UPDATE_TICK) {
            control_.EnableDetailedResults();
            PrintNodes(stat, time_since_start);
            while (time_since_start >= nodes_update_timer + NODES_UPDATE_TICK) {
                nodes_update_timer += NODES_UPDATE_TICK;
            }
        }
    }
    if (q_core::IsMoveNull(final_result.best_move)) {
        final_result.best_move = random_move;
    }
    PrintBestMove(final_result.best_move);
    search_thread.join();
}

void SearchLauncher::Stop() { control_.Stop(); }

void SearchLauncher::Join() {
    if (thread_.joinable()) {
        control_.Stop();
        thread_.join();
    }
}

void SearchLauncher::NewGame() { tt_.NextGame(); }

void SearchLauncher::ChangeTTSize(size_t new_tt_size_mb) {
    tt_ = TranspositionTable(20 + q_util::GetHighestBit(new_tt_size_mb));
}

}  // namespace q_search
