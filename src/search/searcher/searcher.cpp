#include "searcher.h"

#include <cmath>
#include <cstddef>

#include "../../core/moves/attack.h"
#include "core/board/types.h"
#include "core/moves/move.h"
#include "core/util.h"
#include "eval/score.h"
#include "search/control/control.h"
#include "search/position/move_picker.h"
#include "search/position/position.h"
#include "search/position/transposition_table.h"
#include "util/bit.h"

namespace q_search {

Searcher::Searcher(TranspositionTable& tt, RepetitionTable& rt, const Position& position,
                   SearchControl& control, SearchStat& stat)
    : tt_(tt), rt_(rt), position_(position), control_(control), stat_(stat) {
    global_context_.history_table = HistoryTable();
    for (idepth_t i = 0; i < MAX_IDEPTH; i++) {
        global_context_.killer_moves[i] = KillerMoves();
    }
    global_context_.best_move = q_core::NULL_MOVE;
    for (size_t i = 0; i < MAX_IDEPTH; i++) {
        local_context_[i] = LocalContext();
    }
    control_context_.initial_depth = 0;
}

std::vector<q_core::Move> Searcher::GetPV() {
    if (q_core::IsMoveNull(global_context_.best_move)) {
        return {};
    }

    RepetitionTable rt(10);
    Position position = position_;
    q_core::MakeMoveInfo make_move_info;
    q_eval::Evaluator::EvaluatorUpdateInfo evaluator_update_info;
    std::vector<q_core::Move> pv;

    position.MakeMove(global_context_.best_move, make_move_info, evaluator_update_info);
    for (;;) {
        const q_core::hash_t position_hash = position.board.hash;
        if (!rt.Insert(position_hash)) {
            break;
        }
        bool tt_entry_found = false;
        auto* tt_entry = tt_.GetEntry(position_hash, tt_entry_found);
        if (tt_entry_found) {
            const q_core::Move tt_move = q_core::GetDecompressedMove(tt_entry->move);
            if (q_core::IsMovePseudolegal(position.board, tt_move)) {
                if (position.MakeMove(tt_move, make_move_info, evaluator_update_info)) {
                    pv.push_back(tt_move);
                    continue;
                }
            }
        }
        break;
    }
    return pv;
}

SearchResult Searcher::GetSearchResult(depth_t depth, q_eval::score_t score) {
    std::vector<q_core::Move> pv = GetPV();
    return SearchResult{.bound_type = Exact,
                        .score = score,
                        .best_move = global_context_.best_move,
                        .depth = depth,
                        .pv = pv};
}

void Searcher::Run(depth_t max_depth) {
    q_eval::score_t best_score = q_eval::SCORE_UNKNOWN;

    for (uint8_t depth = 1; depth <= max_depth; depth++) {
        q_eval::score_t alpha = q_eval::SCORE_MIN;
        q_eval::score_t beta = q_eval::SCORE_MAX;
        q_eval::score_t delta = 0;
        q_eval::score_t score = q_eval::SCORE_UNKNOWN;

        if (depth > 4) {
            delta = 20;
            alpha = std::max(q_eval::SCORE_MIN, static_cast<q_eval::score_t>(best_score - delta));
            beta = std::min(q_eval::SCORE_MAX, static_cast<q_eval::score_t>(best_score + delta));
        }

        for (;;) {
            if (alpha <= -300) {
                alpha = q_eval::SCORE_MIN;
            }
            if (beta >= 300) {
                beta = q_eval::SCORE_MAX;
            }

            score = RunSearch(depth, alpha, beta);
            if (control_.IsStopped()) {
                break;
            }

            if (score <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(q_eval::SCORE_MIN, static_cast<q_eval::score_t>(score - delta));
            } else if (score >= beta) {
                beta = std::min(q_eval::SCORE_MAX, static_cast<q_eval::score_t>(score + delta));
            } else {
                break;
            }

            delta += delta / 2;
        }
        if (depth > 1 && control_.IsStopped()) {
            break;
        }
        best_score = score;
        if (control_.FinishDepth(depth)) {
            control_.AddResult(GetSearchResult(depth, score));
        }
    }
}

q_eval::score_t Searcher::RunSearch(depth_t depth, q_eval::score_t alpha = q_eval::SCORE_MIN,
                                    q_eval::score_t beta = q_eval::SCORE_MAX) {
    control_context_.initial_depth = depth;
    return Search<NodeType::Root>(depth, 0, alpha, beta);
}

bool Searcher::ShouldStop() { return control_.IsStopped(); }

#define CHECK_STOP \
    if (ShouldStop()) return 0

#define MAKE_MOVE(position, move)                                                   \
    q_core::MakeMoveInfo _make_move_info;                                           \
    q_eval::Evaluator::EvaluatorUpdateInfo _evaluator_update_info;                  \
    bool _legal = position.MakeMove(move, _make_move_info, _evaluator_update_info); \
    if (!_legal) {                                                                  \
        continue;                                                                   \
    }                                                                               \
    Q_DEFER { position.UnmakeMove(move, _make_move_info, _evaluator_update_info); }

const std::array<int16_t, q_core::NUMBER_OF_CELLS> SEE_CELLS_VALUE = {
    0, 100, 300, 300, 500, 900, 3000, 100, 300, 300, 500, 900, 3000};

q_eval::score_t Searcher::QuiescenseSearch(q_eval::score_t alpha, q_eval::score_t beta) {
    CHECK_STOP;
    stat_.IncNodesCount();
    const q_eval::score_t score = position_.GetEvaluatorScore();

    bool in_check = position_.IsCheck();

    if (!in_check) {
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            return beta;
        }
    }
    QuiescenseMovePicker move_picker(position_, in_check);
    size_t moves_done = 0;
    for (q_core::Move move = move_picker.GetNextMove();
         move_picker.GetStage() != QuiescenseMovePicker::Stage::End;
         move = move_picker.GetNextMove()) {
        CHECK_STOP;
        if (!in_check && q_core::IsMoveCapture(move) && !q_eval::IsScoreMate(alpha) &&
            position_.HasNonPawns()) {
            if (!q_core::IsSEENotNegative(position_.board, move, 0, SEE_CELLS_VALUE)) {
                continue;
            }
        }
        MAKE_MOVE(position_, move);
        moves_done++;
        q_eval::score_t new_score = -QuiescenseSearch(-beta, -alpha);
        alpha = std::max(alpha, new_score);
        if (alpha >= beta) {
            return beta;
        }
    }
    if (in_check && moves_done == 0) {
        return q_eval::SCORE_ALMOST_MATE;
    }
    return alpha;
}

q_eval::score_t AdjustCheckmate(const q_eval::score_t score, depth_t depth) {
    if (q_eval::IsScoreMate(score)) {
        return score > 0 ? score - depth : score + depth;
    }
    return score;
}

#define SAVE_ROOT_BEST_MOVE \
    if constexpr (node_type == NodeType::Root) global_context_.best_move = best_move;

#define SEND_ROOT_LOWERBOUND                                        \
    if constexpr (node_type == NodeType::Root)                      \
    control_.AddResult(SearchResult{.bound_type = Lower,            \
                                    .score = std::min(alpha, beta), \
                                    .best_move = best_move,         \
                                    .depth = depth,                 \
                                    .pv = {}})

#define SEND_ROOT_UPPERBOUND                   \
    if constexpr (node_type == NodeType::Root) \
    control_.AddResult(SearchResult{           \
        .bound_type = Upper, .score = alpha, .best_move = best_move, .depth = depth, .pv = {}})

#define SEND_ROOT_MOVE                         \
    if constexpr (node_type == NodeType::Root) \
    control_.AddRootMove(RootMove{.depth = depth, .move = move, .number = moves_done})

std::array<std::array<depth_t, 64>, 32> GetLMRDepthReduction() {
    std::array<std::array<depth_t, 64>, 32> res{};
    for (size_t depth = 0; depth < 32; depth++) {
        for (size_t move = 0; move < 64; move++) {
            if (depth < 3 || move < 3) {
                res[depth][move] = 1;
                continue;
            }
            depth_t reduction = std::log(depth) * std::log(std::log(move)) + 2;
            res[depth][move] = reduction;
        }
    }
    return res;
}

inline static constexpr uint8_t FIFTY_MOVES_RULE_LIMIT = 100;
inline static constexpr uint8_t FIFTY_MOVES_RULE_HASH_TABLE_LIMIT = FIFTY_MOVES_RULE_LIMIT - 10;

inline static constexpr depth_t FPR_DEPTH_THRESHOLD = 2;
inline static constexpr std::array<depth_t, FPR_DEPTH_THRESHOLD + 1> FPR_MARGIN = {0, 50, 125};

inline static constexpr depth_t RPR_DEPTH_THRESHOLD = 2;
inline static constexpr std::array<depth_t, RPR_DEPTH_THRESHOLD + 1> RPR_MARGIN = {0, 50, 125};

inline static constexpr depth_t LMR_DEPTH_THRESHOLD = 3;
inline static const std::array<std::array<depth_t, 64>, 32> LMR_DEPTH_REDUCTION =
    GetLMRDepthReduction();

template <Searcher::NodeType node_type>
q_eval::score_t Searcher::Search(depth_t depth, idepth_t idepth, q_eval::score_t alpha,
                                 q_eval::score_t beta) {
    CHECK_STOP;

    // Checking fifty move rule
    if (position_.board.fifty_rule_move_count >= FIFTY_MOVES_RULE_LIMIT) {
        return 0;
    }

    // Checking repetition table
    const q_core::hash_t position_hash = position_.board.hash;
    if (!rt_.Insert(position_hash)) {
        return 0;
    }
    Q_DEFER { rt_.Erase(position_hash); };

    // Performing quiescense search
    if (depth <= 0) {
        if (alpha >= -q_eval::SCORE_ALMOST_MATE) {
            return alpha;
        }
        if (beta <= q_eval::SCORE_ALMOST_MATE) {
            return beta;
        }
        return QuiescenseSearch(alpha, beta);
    }

    // Mate pruning
    if constexpr (node_type != NodeType::Root) {
        alpha = std::max(alpha, static_cast<q_eval::score_t>(q_eval::SCORE_MATE + idepth));
        beta = std::min(beta, static_cast<q_eval::score_t>(-(q_eval::SCORE_MATE + idepth + 1)));
        if (alpha >= beta) {
            return beta;
        }
    }

    // Increase nodes count
    stat_.IncNodesCount();

    // Prepare for search
    const q_eval::score_t initial_alpha = alpha;
    const q_eval::score_t initial_beta = beta;

    // Prepare local context
    local_context_[idepth].current_move = q_core::NULL_MOVE;
    local_context_[idepth].eval = q_eval::SCORE_UNKNOWN;

    auto get_node_evaluation = [&]() {
        if (local_context_[idepth].eval == q_eval::SCORE_UNKNOWN) {
            local_context_[idepth].eval = position_.GetEvaluatorScore();
        }
    };

    // Checking transposition table
    q_core::Move tt_move = q_core::NULL_MOVE;
    bool tt_entry_found = false;
    TranspositionTable::Entry* tt_entry = nullptr;
    if (IsMoveNull(local_context_[idepth].skip_move)) {
        tt_entry = tt_.GetEntry(position_hash, tt_entry_found);
    }

    auto tt_store_move = [&](q_eval::score_t score, const q_core::Move best_move) {
        if (tt_entry) {
            auto tt_node_type = TranspositionTable::NodeType::ExactValue;
            if (score <= initial_alpha) {
                score = initial_alpha;
                tt_node_type = TranspositionTable::NodeType::UpperBound;
            }
            if (score >= initial_beta) {
                score = initial_beta;
                tt_node_type = TranspositionTable::NodeType::LowerBound;
            }
            score = AdjustCheckmate(score, -static_cast<depth_t>(idepth));
            if (!IsMoveNull(best_move)) {
                tt_.Store(*tt_entry, position_hash, best_move, score, depth, tt_node_type,
                          node_type != NodeType::Simple);
            }
        }
    };

    if (tt_entry_found) {
        tt_move = q_core::GetDecompressedMove(tt_entry->move);
        const bool is_cutoff_allowed =
            (node_type != NodeType::Root) & (tt_entry->depth >= depth) &
            (position_.board.fifty_rule_move_count < FIFTY_MOVES_RULE_HASH_TABLE_LIMIT) &
            (node_type == NodeType::Simple ||
             tt_entry->info.GetGeneration() == tt_.GetGeneration());
        if (is_cutoff_allowed) {
            const q_eval::score_t score = AdjustCheckmate(tt_entry->score, idepth);
            const auto tt_node_type = tt_entry->info.GetNodeType();
            const bool is_cutoff =
                (tt_node_type == TranspositionTable::NodeType::ExactValue) ||
                (tt_node_type == TranspositionTable::NodeType::LowerBound && score >= beta) ||
                (tt_node_type == TranspositionTable::NodeType::UpperBound && score <= alpha);
            if (is_cutoff) {
                return score;
            }
        }
    }

    const bool is_check = position_.IsCheck();

    // Futility pruning
    if (node_type == NodeType::Simple && depth <= FPR_DEPTH_THRESHOLD &&
        !q_eval::IsScoreMate(beta) && !is_check) {
        get_node_evaluation();
        if (local_context_[idepth].eval >= beta + FPR_MARGIN[depth]) {
            return beta;
        }
    }

    // Razoring
    if (node_type == NodeType::Simple && depth <= RPR_DEPTH_THRESHOLD &&
        !q_eval::IsScoreMate(alpha)) {
        get_node_evaluation();
        q_eval::score_t threshold = alpha - RPR_MARGIN[depth];
        if (local_context_[idepth].eval <= threshold) {
            return QuiescenseSearch(alpha, beta);
        }
    }

    // Null move pruning
    if (node_type == NodeType::Simple && !is_check &&
        !IsMoveNull(local_context_[idepth - 1].current_move) &&
        IsMoveNull(local_context_[idepth].skip_move) &&
        position_.HasNonPawns(position_.board.move_side) && depth > 1) {
        get_node_evaluation();
        if (local_context_[idepth].eval >= beta) {
            q_core::coord_t old_en_passant_coord;
            position_.MakeNullMove(old_en_passant_coord);
            Q_DEFER { position_.UnmakeNullMove(old_en_passant_coord); };

            const q_eval::score_t new_score =
                -Search<NodeType::Simple>(depth - 3 - depth / 4, idepth + 1, -beta, -beta + 1);
            CHECK_STOP;
            if (new_score >= beta) {
                return beta;
            }
        }
    }

    // Try moves one by one
    MovePicker move_picker(position_, tt_move, global_context_.killer_moves[idepth],
                           global_context_.history_table);
    q_core::Move best_move = q_core::NULL_MOVE;
    size_t moves_done = 0;
    size_t history_moves_done = 0;
    for (q_core::Move move = move_picker.GetNextMove();
         move_picker.GetStage() != MovePicker::Stage::End; move = move_picker.GetNextMove()) {
            CHECK_STOP;
        if (move == local_context_[idepth].skip_move) {
            continue;
        }

        q_core::cell_t src_cell = position_.board.cells[move.src];
        MAKE_MOVE(position_, move);
        SEND_ROOT_MOVE;

        moves_done++;
        history_moves_done += move_picker.GetStage() == MovePicker::Stage::History ? 1 : 0;
        q_eval::score_t score = q_eval::SCORE_UNKNOWN;
        local_context_[idepth].current_move = move;

        // Late move reduction
        if (node_type == NodeType::Simple && move_picker.GetStage() >= MovePicker::Stage::History &&
            depth >= LMR_DEPTH_THRESHOLD && moves_done > 1 && !position_.IsCheck()) {
            depth_t depth_reduction =
                LMR_DEPTH_REDUCTION[std::min(depth, static_cast<depth_t>(31))]
                                   [std::min(static_cast<size_t>(63), history_moves_done)];

            depth_reduction = std::min(static_cast<depth_t>(depth - 1),
                                       std::max(depth_reduction, static_cast<depth_t>(1)));
            score =
                -Search<NodeType::Simple>(depth - depth_reduction, idepth + 1, -alpha - 1, -alpha);
            if (score > alpha && depth_reduction > 1) {
                score = -Search<NodeType::Simple>(depth - 1, idepth + 1, -alpha - 1, -alpha);
            }
        } else if (node_type == NodeType::Simple || moves_done > 1) {
            score = -Search<NodeType::Simple>(depth - 1, idepth + 1, -alpha - 1, -alpha);
        }
        if (node_type != NodeType::Simple &&
            (moves_done == 1 || (score > alpha && (node_type == NodeType::Root || score < beta)))) {
            score = -Search<NodeType::PV>(depth - 1, idepth + 1, -beta, -alpha);
        }

        if (score > alpha) {
            alpha = score;
            best_move = move;
            if (depth == 1) {
                SAVE_ROOT_BEST_MOVE;
            }
            SEND_ROOT_LOWERBOUND;
        }
        if (alpha >= beta) {
            SAVE_ROOT_BEST_MOVE;
            if (IsMoveNull(local_context_[idepth].skip_move)) {
                tt_store_move(beta, best_move);
                if (move_picker.GetStage() >= MovePicker::Stage::KillerMoves) {
                    global_context_.killer_moves[idepth].Add(best_move);
                    global_context_.history_table.Update(src_cell, best_move, depth);
                }
            }
            return beta;
        }
    }
    if (moves_done == 0) {
        if (q_core::IsKingInCheck(position_.board)) {
            return IsMoveNull(local_context_[idepth].skip_move) ? q_eval::SCORE_MATE + idepth
                                                                : alpha;
        }
        return 0;
    }
    SAVE_ROOT_BEST_MOVE;
    if (q_core::IsMoveNull(best_move)) {
        SEND_ROOT_UPPERBOUND;
    }
    if (IsMoveNull(local_context_[idepth].skip_move)) {
        tt_store_move(alpha, best_move);
    }
    return alpha;
}

template q_eval::score_t Searcher::Search<Searcher::NodeType::Root>(depth_t depth, idepth_t idepth,
                                                                    q_eval::score_t alpha,
                                                                    q_eval::score_t beta);
template q_eval::score_t Searcher::Search<Searcher::NodeType::PV>(depth_t depth, idepth_t idepth,
                                                                  q_eval::score_t alpha,
                                                                  q_eval::score_t beta);
template q_eval::score_t Searcher::Search<Searcher::NodeType::Simple>(depth_t depth,
                                                                      idepth_t idepth,
                                                                      q_eval::score_t alpha,
                                                                      q_eval::score_t beta);

}  // namespace q_search
