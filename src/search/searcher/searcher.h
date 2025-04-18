#ifndef QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H

#include <vector>

#include "../control/control.h"
#include "../control/stat.h"
#include "../position/move_picker.h"
#include "../position/position.h"
#include "../position/repetition_table.h"
#include "../position/transposition_table.h"
#include "core/moves/move.h"

namespace q_search {

class Searcher {
  public:
    Searcher(TranspositionTable& tt, RepetitionTable& rt, const Position& position,
             SearchControl& control, SearchStat& stat);
    void Run(depth_t max_depth);
    static constexpr depth_t MAX_DEPTH = 127;

  private:
    enum class NodeType { Root, PV, Simple };
    q_eval::score_t QuiescenseSearch(q_eval::score_t alpha, q_eval::score_t beta);
    q_eval::score_t RunSearch(depth_t depth, q_eval::score_t alpha, q_eval::score_t beta);
    template <NodeType node_type>
    q_eval::score_t Search(depth_t depth, idepth_t idepth, q_eval::score_t alpha,
                           q_eval::score_t beta);

    SearchResult GetSearchResult(depth_t depth, q_eval::score_t score);
    std::vector<q_core::Move> GetPV();
    bool ShouldStop();

    static constexpr idepth_t MAX_IDEPTH = 255;
    struct GlobalContext {
        HistoryTable history_table;
        KillerMoves killer_moves[MAX_IDEPTH];
        q_core::Move best_move;
    };
    struct LocalContext {
        q_core::Move current_move;
        q_eval::score_t eval;
        q_core::Move skip_move = q_core::NULL_MOVE;
    };
    struct ControlContext {
        depth_t initial_depth;
    };

    TranspositionTable& tt_;
    RepetitionTable& rt_;
    Position position_;
    SearchControl& control_;
    SearchStat& stat_;
    GlobalContext global_context_;
    LocalContext local_context_[MAX_IDEPTH];
    ControlContext control_context_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
