#include "searcher.h"

#include "../position/move_picker.h"
#include "../../core/moves/attack.h"

namespace q_search {

Searcher::Searcher(TranspositionTable& tt, RepetitionTable& rt, const Position& position,
                   SearchControl& control, SearchStat& stat)
    : tt_(tt), rt_(rt), position_(position), control_(control), stat_(stat) {}

SearchResult Searcher::GetSearchResult(depth_t depth, q_eval::score_t score) {
    std::vector<q_core::Move> pv;
    return SearchResult{.bound_type = Exact,
                        .score = score,
                        .best_move = best_move_,
                        .depth = depth,
                        .pv = pv};
}

void Searcher::Run(depth_t max_depth) {
    for (uint8_t depth = 1; depth <= max_depth; depth++) {
        q_eval::score_t score = Search<NodeType::Root>(depth, 0, q_eval::SCORE_MIN, q_eval::SCORE_MAX);
        if (control_.IsStopped()) {
            break;
        }
        if (control_.FinishDepth(depth)) {
            control_.AddResult(GetSearchResult(depth, score));
        }
    }
}

bool Searcher::ShouldStop() {
    return control_.IsStopped();
}

#define CHECK_STOP \
    if (ShouldStop()) { \
        return 0; \
    } \

#define MAKE_MOVE(position, move) \
    q_core::MakeMoveInfo _make_move_info; \
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag _evaluator_tag; \
    bool _legal = position.MakeMove(move, _make_move_info, _evaluator_tag); \
    if (!_legal) { \
        continue; \
    } \
    Q_DEFER { position.UnmakeMove(move, _make_move_info, _evaluator_tag); }

q_eval::score_t Searcher::QuiescenseSearch(q_eval::score_t alpha, q_eval::score_t beta) {
    CHECK_STOP;
    stat_.IncNodesCount();
    const q_eval::score_t score = position_.GetEvaluatorScore();
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
        return beta;
    }
    MovePicker move_picker(position_);
    for (q_core::Move move = move_picker.GetNextMove(); move_picker.GetStage() <= MovePicker::Stage::Promotion; move = move_picker.GetNextMove()) {
        CHECK_STOP;
        Q_ASSERT(q_core::IsMoveEnPassant(move) || q_core::IsMovePromotion(move) || position_.board.cells[move.dst] == q_core::EMPTY_CELL);
        MAKE_MOVE(position_, move);
        q_eval::score_t new_score = -QuiescenseSearch(-beta, -alpha);
        alpha = std::max(alpha, new_score);
        if (alpha >= beta) {
            return beta;
        }
    }
    return alpha;
}

template <Searcher::NodeType node_type> 
q_eval::score_t Searcher::Search(depth_t depth, idepth_t idepth, q_eval::score_t alpha, q_eval::score_t beta) {
    CHECK_STOP;
    // Checking repetition table
    const q_core::hash_t position_hash = position_.board.hash;
    if (!rt_.Insert(position_hash)) {
        return 0;
    }
    Q_DEFER { rt_.Erase(position_hash); };

    // Performing quiescense search
    if (depth <= 0) {
        q_eval::score_t score = QuiescenseSearch(alpha, beta);
        return score;
    }

    stat_.IncNodesCount();
    
    // Try moves one by one
    MovePicker move_picker(position_);
    size_t moves_done = 0;
    for (q_core::Move move = move_picker.GetNextMove(); move_picker.GetStage() != MovePicker::Stage::End; move = move_picker.GetNextMove()) {
        CHECK_STOP;
        MAKE_MOVE(position_, move);
        moves_done++;
        const q_eval::score_t new_score = -Search<node_type>(depth - 1, idepth + 1, -beta, -alpha);
        if (new_score > alpha) {
            alpha = new_score;
            if (idepth == 0) {
                best_move_ = move;
            }
        }
        if (alpha >= beta) {
            return beta;
        }
    }
    if (moves_done == 0) {
        if (q_core::IsKingInCheck(position_.board)) {
            return q_eval::SCORE_MATE + idepth;
        }
        return 0;
    }
    return alpha;
}

template q_eval::score_t Searcher::Search<Searcher::NodeType::Root>(depth_t depth, idepth_t idepth, q_eval::score_t alpha, q_eval::score_t beta);
template q_eval::score_t Searcher::Search<Searcher::NodeType::PV>(depth_t depth, idepth_t idepth, q_eval::score_t alpha, q_eval::score_t beta);
template q_eval::score_t Searcher::Search<Searcher::NodeType::Simple>(depth_t depth, idepth_t idepth, q_eval::score_t alpha, q_eval::score_t beta);

}  // namespace q_search
