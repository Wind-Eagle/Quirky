#ifndef QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H

#include <vector>

#include "../control/control.h"
#include "../position/transposition_table.h"
#include "../position/position.h"
#include "../position/repetition_table.h"
#include "../control/stat.h"

namespace q_search {

class Searcher {
    public:
        Searcher(TranspositionTable& tt, RepetitionTable& rt, const Position& position, SearchControl& control, SearchStat& stat);
        void Run(depth_t max_depth);
        static constexpr depth_t MAX_DEPTH = 127;
    private:
        SearchResult GetSearchResult(depth_t depth);
        void Search(depth_t depth);
        TranspositionTable& tt_;
        RepetitionTable& rt_;
        Position position_;
        SearchControl& control_;
        SearchStat& stat_;
        q_core::Move best_move_;
        q_eval::score_t best_score_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
