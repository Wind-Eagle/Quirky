#include "searcher.h"

#include "../../core/moves/attack.h"

namespace q_search {

Searcher::Searcher(TranspositionTable& tt, RepetitionTable& rt, const Position& position,
                   SearchControl& control, SearchStat& stat)
    : tt_(tt), rt_(rt), position_(position), control_(control), stat_(stat) {
    global_context_.history_table = HistoryTable();
    for (idepth_t i = 0; i < MAX_IDEPTH; i++) {
        global_context_.killer_moves[i] = KillerMoves();
    }
    global_context_.best_move = q_core::NULL_MOVE;
}

std::vector<q_core::Move> Searcher::GetPV() {
    if (q_core::IsMoveNull(global_context_.best_move)) {
        return {};
    }

    RepetitionTable rt(10);
    Position position = position_;
    q_core::MakeMoveInfo make_move_info;
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag evaluator_tag;
    std::vector<q_core::Move> pv;

    position.MakeMove(global_context_.best_move, make_move_info, evaluator_tag);
    for (;;) {
        const q_core::hash_t position_hash = position.board.hash;
        if (!rt.Insert(position_hash)) {
            break;
        }
        bool tt_entry_found = false;
        auto& tt_entry = tt_.GetEntry(position_hash, tt_entry_found);
        if (tt_entry_found) {
            const q_core::Move tt_move = q_core::GetDecompressedMove(tt_entry.move);
            if (q_core::IsMovePseudolegal(position.board, tt_move)) {
                if (position.MakeMove(tt_move, make_move_info, evaluator_tag)) {
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
    for (uint8_t depth = 1; depth <= max_depth; depth++) {
        q_eval::score_t score =
            Search<NodeType::Root>(depth, 0, q_eval::SCORE_MIN, q_eval::SCORE_MAX);
        if (control_.IsStopped()) {
            break;
        }
        if (control_.FinishDepth(depth)) {
            control_.AddResult(GetSearchResult(depth, score));
        }
    }
}

bool Searcher::ShouldStop() { return control_.IsStopped(); }

#define CHECK_STOP \
    if (ShouldStop()) return 0

#define MAKE_MOVE(position, move)                                           \
    q_core::MakeMoveInfo _make_move_info;                                   \
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag _evaluator_tag;   \
    bool _legal = position.MakeMove(move, _make_move_info, _evaluator_tag); \
    if (!_legal) {                                                          \
        continue;                                                           \
    }                                                                       \
    Q_DEFER { position.UnmakeMove(move, _make_move_info, _evaluator_tag); }

q_eval::score_t Searcher::QuiescenseSearch(q_eval::score_t alpha, q_eval::score_t beta) {
    CHECK_STOP;
    stat_.IncNodesCount();
    const q_eval::score_t score = position_.GetEvaluatorScore();
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
        return beta;
    }
    QuiescenseMovePicker move_picker(position_);
    for (q_core::Move move = move_picker.GetNextMove();
         move_picker.GetStage() != QuiescenseMovePicker::Stage::End;
         move = move_picker.GetNextMove()) {
        CHECK_STOP;
        Q_ASSERT(q_core::IsMoveEnPassant(move) || q_core::IsMovePromotion(move) ||
                 position_.board.cells[move.dst] != q_core::EMPTY_CELL);
        MAKE_MOVE(position_, move);
        q_eval::score_t new_score = -QuiescenseSearch(-beta, -alpha);
        alpha = std::max(alpha, new_score);
        if (alpha >= beta) {
            return beta;
        }
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
    if constexpr (node_type == NodeType::Root) global_context_.best_move = best_move

inline static constexpr uint8_t FIFTY_MOVES_RULE_LIMIT = 100;
inline static constexpr uint8_t FIFTY_MOVES_RULE_HASH_TABLE_LIMIT = FIFTY_MOVES_RULE_LIMIT - 10;

inline static constexpr depth_t NMP_DEPTH_THRESHOLD = 5;
inline static constexpr depth_t NMP_DEPTH_REDUCTION = 2;

template <Searcher::NodeType node_type>
q_eval::score_t Searcher::Search(depth_t depth, idepth_t idepth, q_eval::score_t alpha,
                                 q_eval::score_t beta) {
    CHECK_STOP;
    // Checking repetition table
    const q_core::hash_t position_hash = position_.board.hash;
    if (!rt_.Insert(position_hash)) {
        return 0;
    }
    Q_DEFER { rt_.Erase(position_hash); };

    // Prepare for search
    const q_eval::score_t initial_alpha = alpha;
    const q_eval::score_t initial_beta = beta;

    // Performing quiescense search
    if (depth <= 0) {
        return QuiescenseSearch(alpha, beta);
    }

    // Increase nodes count
    stat_.IncNodesCount();

    // Checking transposition table
    q_core::Move tt_move = q_core::NULL_MOVE;
    bool tt_entry_found = false;
    auto& tt_entry = tt_.GetEntry(position_hash, tt_entry_found);

    auto tt_store_move = [&](q_eval::score_t score, const q_core::Move best_move) {
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
            tt_.Store(tt_entry, position_hash, best_move, score, depth, tt_node_type,
                      node_type != NodeType::Simple);
        }
    };

    if (tt_entry_found) {
        tt_move = q_core::GetDecompressedMove(tt_entry.move);
        const bool is_cutoff_allowed =
            (node_type != NodeType::Root) & (tt_entry.depth >= depth) &
            (position_.board.fifty_rule_move_count < FIFTY_MOVES_RULE_HASH_TABLE_LIMIT) &
            (node_type == NodeType::Simple || tt_entry.info.GetGeneration() == tt_.GetGeneration());
        if (is_cutoff_allowed) {
            const q_eval::score_t score = AdjustCheckmate(tt_entry.score, idepth);
            const auto tt_node_type = tt_entry.info.GetNodeType();
            const bool is_cutoff =
                (tt_node_type == TranspositionTable::NodeType::ExactValue) ||
                (tt_node_type == TranspositionTable::NodeType::LowerBound && score >= beta) ||
                (tt_node_type == TranspositionTable::NodeType::UpperBound && score <= alpha);
            if (is_cutoff) {
                return score;
            }
        }
    }

    // Prepare local context
    local_context_[idepth].current_move = q_core::NULL_MOVE;

    // Null move pruning
    if (node_type == NodeType::Simple && depth >= NMP_DEPTH_THRESHOLD && !q_core::IsKingInCheck(position_.board) &&
        !q_eval::IsScoreMate(alpha) && !q_eval::IsScoreMate(beta) &&
        !IsMoveNull(local_context_[idepth - 1].current_move) &&
        !IsMoveCapture(local_context_[idepth - 1].current_move)) {
        q_core::coord_t old_en_passant_coord;
        position_.MakeNullMove(old_en_passant_coord);
        Q_DEFER { position_.UnmakeNullMove(old_en_passant_coord); };

        const q_eval::score_t new_score = -Search<NodeType::Simple>(depth - NMP_DEPTH_REDUCTION - 1, idepth + 1, -beta, -beta + 1);
        if (new_score >= beta) {
            return beta;
        }
    }

    // Try moves one by one
    MovePicker move_picker(position_, tt_move, global_context_.killer_moves[idepth],
                           global_context_.history_table);
    q_core::Move best_move = q_core::NULL_MOVE;
    size_t moves_done = 0;
    for (q_core::Move move = move_picker.GetNextMove();
         move_picker.GetStage() != MovePicker::Stage::End; move = move_picker.GetNextMove()) {
        CHECK_STOP;
        q_core::cell_t src_cell = position_.board.cells[move.src];
        MAKE_MOVE(position_, move);

        local_context_[idepth].current_move = move;
        const bool do_pv_search = (node_type != NodeType::Simple) & (moves_done > 0);
        moves_done++;
        q_eval::score_t new_score = alpha;
        if (do_pv_search) {
            new_score = -Search<NodeType::Simple>(depth - 1, idepth + 1, -alpha - 1, -alpha);
            new_score = (new_score <= alpha ? alpha : alpha + 1);
        }
        if (!do_pv_search || (alpha < new_score && new_score < beta)) {
            const NodeType new_node_type =
                (node_type == NodeType::Simple ? NodeType::Simple : NodeType::PV);
            new_score = -Search<new_node_type>(depth - 1, idepth + 1, -beta, -alpha);
        }

        if (new_score > alpha) {
            alpha = new_score;
            best_move = move;
            if (depth == 1) {
                SAVE_ROOT_BEST_MOVE;
            }
        }
        if (alpha >= beta) {
            SAVE_ROOT_BEST_MOVE;
            tt_store_move(beta, best_move);
            if (move_picker.GetStage() >= MovePicker::Stage::KillerMoves) {
                global_context_.killer_moves[idepth].Add(best_move);
                global_context_.history_table.Update(src_cell, best_move, depth);
            }
            return beta;
        }
    }
    if (moves_done == 0) {
        if (q_core::IsKingInCheck(position_.board)) {
            return q_eval::SCORE_MATE + idepth;
        }
        return 0;
    }
    SAVE_ROOT_BEST_MOVE;
    tt_store_move(alpha, best_move);
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
