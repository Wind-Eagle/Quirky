#ifndef QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H

#include <vector>

#include "core/moves/move.h"
#include "search/control/control.h"
#include "search/control/stat.h"
#include "search/position/move_picker.h"
#include "search/position/position.h"
#include "search/position/repetition_table.h"
#include "search/position/transposition_table.h"

namespace q_search {

class Searcher {
  public:
    Searcher(TranspositionTable& tt, RepetitionTable& rt, const q_core::Board& board,
             SearchControl& control, SearchStat& stat);
    void Run(depth_t max_depth, size_t pv_count);

    static constexpr depth_t MAX_DEPTH = (Position::MAX_BUFFER_SIZE - 1) / 2;

  private:
    enum class NodeType { Root, PV, Simple };
    q_eval::score_t QuiescenseSearch(q_eval::score_t alpha, q_eval::score_t beta);
    q_eval::score_t RunSearch(depth_t depth, q_eval::score_t alpha, q_eval::score_t beta);
    template <NodeType node_type>
    q_eval::score_t Search(depth_t depth, idepth_t idepth, q_eval::score_t alpha,
                           q_eval::score_t beta, bool is_cut_node);

    SearchResult GetSearchResult(RootMoveWithScore result);
    std::vector<q_core::Move> GetPV(q_core::Move best_move);
    bool ShouldStop();

    static constexpr idepth_t MAX_IDEPTH = Position::MAX_BUFFER_SIZE - 1;
    struct GlobalContext {
        HistoryTable history_table;
        q_core::MoveList root_forbidden_moves;
        size_t pv_count;
        q_core::Move best_move;
        depth_t initial_depth;
        idepth_t nmp_min_idepth;
    };
    struct LocalContext {
        StatefulMove current_move;
        q_eval::score_t eval;
        q_core::Move skip_move = q_core::NULL_MOVE;
        bool nmp_verification = false;
    };

    TranspositionTable& tt_;
    RepetitionTable& rt_;
    Position position_;
    SearchControl& control_;
    SearchStat& stat_;
    GlobalContext global_context_;
    LocalContext local_context_[MAX_IDEPTH];
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
