#include "launcher.h"

#include <utility>
#include <fstream>

#include "../../util/string.h"
#include "../control/control.h"
#include "../control/stat.h"
#include "searcher.h"

namespace q_search {

uint8_t GetRTByteSizeLog(size_t moves_count) {
    return q_util::GetHighestBit((moves_count + Searcher::MAX_DEPTH) * 4) + 3;
}

void ProcessPositionMoves(Position& position, const std::vector<q_core::Move>& moves,
                          RepetitionTable& rt) {
    RepetitionTable helper_rt{GetRTByteSizeLog(moves.size())};
    helper_rt.Insert(position.board.hash);
    for (const auto& move : moves) {
        if (helper_rt.Has(position.board.hash)) {
            rt.Insert(position.board.hash);
        } else {
            helper_rt.Insert(position.board.hash);
        }
        q_core::MakeMoveInfo make_move_info;
        q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag evaluator_tag;
        position.MakeMove(move, make_move_info, evaluator_tag);
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

void PrintSearchResult(const SearchResult& result, time_t time_since_start) {
    std::vector<std::string> moves;
    moves.push_back(q_core::CastMoveToString(result.best_move));
    for (const auto& move : result.pv) {
        moves.push_back(q_core::CastMoveToString(move));
    }
    std::string pv_str = q_util::ConcatenateStrings(moves.begin(), moves.end());
    q_util::Print("info depth", static_cast<int>(result.depth), "time", time_since_start, "score cp", result.score,
                  "pv", pv_str);
}

void PrintNodes(const SearchStat& stat) { q_util::Print("info nodes", stat.GetNodesCount()); }

void PrintBestMove(const q_core::Move move) {
    q_util::Print("bestmove", q_core::CastMoveToString(move));
}

void SearchLauncher::StartMainThread(const Position& start_position,
                                     const std::vector<q_core::Move>& moves,
                                     time_control_t time_control, depth_t max_depth) {
    RepetitionTable rt{GetRTByteSizeLog(moves.size())};
    Position position = start_position;
    ProcessPositionMoves(position, moves, rt);
    SearchStat stat;
    Searcher searcher(tt_, rt, position, control_, stat);
    SearchTimer timer(time_control, control_, stat);
    std::thread search_thread = std::thread([&]() { searcher.Run(max_depth); });

    SearchResult final_result{.best_move = q_core::NULL_MOVE, .depth = 0};
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
            std::vector<SearchResult> results = std::move(control_.GetResults());
            for (auto& result : results) {
                PrintSearchResult(result, time_since_start);
                if (result.bound_type == Exact && result.depth > final_result.depth) {
                    final_result = std::move(result);
                }
            }
            if (final_result.depth >= max_depth) {
                control_.Stop();
            }
        }

        if (time_left == std::chrono::milliseconds(0)) {
            control_.Stop();
        }
        if (time_since_start >= nodes_update_timer + NODES_UPDATE_TICK) {
            PrintNodes(stat);
            while (time_since_start >= nodes_update_timer + NODES_UPDATE_TICK) {
                nodes_update_timer += NODES_UPDATE_TICK;
            }
        }
    }
    if (q_core::IsMoveNull(final_result.best_move)) {
        q_core::MoveList move_list;
        q_core::GenerateAllSimpleMoves(position.board, move_list);
        final_result.best_move = move_list.moves[0];
    }
    if (!q_core::IsMovePseudolegal(position.board, final_result.best_move)) {
        std::cerr << position.board.GetFEN() << " " << q_core::CastMoveToString(final_result.best_move) << std::endl;
        exit(42);
    }
    PrintBestMove(final_result.best_move);
    search_thread.join();
}

void SearchLauncher::Stop() {
    control_.Stop();
}

void SearchLauncher::Join() {
    if (thread_.joinable()) {
        control_.Stop();
        thread_.join();
    }
}

void SearchLauncher::NewGame() { tt_.NextGame(); }

}  // namespace q_search
