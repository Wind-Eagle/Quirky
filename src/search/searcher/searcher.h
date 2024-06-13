#ifndef QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H

#include <vector>

#include "../control/control.h"
#include "../position/move_picker.h"
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
        enum class NodeType {
            Root,
            PV,
            Simple
        };
        q_eval::score_t QuiescenseSearch(q_eval::score_t alpha, q_eval::score_t beta);
        template <NodeType node_type>
        q_eval::score_t Search(depth_t depth, idepth_t idepth, q_eval::score_t alpha, q_eval::score_t beta);
        SearchResult GetSearchResult(depth_t depth, q_eval::score_t score);
        bool ShouldStop();

        static constexpr idepth_t MAX_IDEPTH = 255;
        struct Context {
            std::array<q_core::Move, MovePicker::KILLER_MOVES_COUNT> killer_moves[MAX_IDEPTH];
            q_core::Move best_move;
        };

        TranspositionTable& tt_;
        RepetitionTable& rt_;
        Position position_;
        SearchControl& control_;
        SearchStat& stat_;
        Context context_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
