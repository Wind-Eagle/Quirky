#include "searcher.h"

namespace q_search {

Searcher::Searcher(TranspositionTable& tt, RepetitionTable& rt, const Position& position,
                   SearchControl& control, SearchStat& stat)
    : tt_(tt), rt_(rt), position_(position), control_(control), stat_(stat) {}

SearchResult Searcher::GetSearchResult(depth_t depth) {
    std::vector<q_core::Move> pv;
    return SearchResult{.bound_type = Exact,
                        .score = best_score_,
                        .best_move = best_move_,
                        .depth = depth,
                        .pv = pv};
}

void Searcher::Run(depth_t max_depth) {
    for (uint8_t depth = 1; depth <= max_depth; depth++) {
        Search(depth);
        if (control_.IsStopped()) {
            break;
        }
        if (control_.FinishDepth(depth)) {
            control_.AddResult(GetSearchResult(depth));
        }
    }
}

void Searcher::Search(depth_t depth) {
    for (size_t i = 0; i < (1ULL << (20 + depth)); i++) {
        stat_.IncNodesCount();
    }
    q_core::MoveList move_list;
    q_core::GenerateAllMoves(position_.board, move_list);
    best_move_ = move_list.moves[0];
    best_score_ = depth * 10;
}

}  // namespace q_search
