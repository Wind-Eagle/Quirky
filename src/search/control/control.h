#ifndef QUIRKY_SRC_SEARCH_CONTROL_CONTROL_H
#define QUIRKY_SRC_SEARCH_CONTROL_CONTROL_H

#include <condition_variable>
#include <vector>

#include "../../core/moves/move.h"
#include "../../eval/score.h"
#include "../position/position.h"
#include "stat.h"

namespace q_search {

enum SearchResultBoundType { Exact, Lower, Upper };

struct SearchResult {
    SearchResultBoundType bound_type;
    q_eval::score_t score;
    q_core::Move best_move;
    depth_t depth;
    std::vector<q_core::Move> pv;
};

struct RootMove {
    depth_t depth;
    q_core::Move move;
    size_t number;
};

class SearchControl {
  public:
    enum class Event { Timeout, Stop, NewResult, RootMove };
    void Stop();
    template <class Rep, class Period>
    Event Wait(const std::chrono::duration<Rep, Period> time) {
        std::unique_lock guard(lock_);
        if (auto event = GetEvent(); event != Event::Timeout) {
            return event;
        }
        event_.wait_for(guard, time);
        return GetEvent();
    }
    void Reset();
    void EnableDetailedResults();
    bool AreDetailedResultsEnabled();
    bool IsStopped() const;
    depth_t GetDepth() const;
    bool FinishDepth(depth_t depth);
    void AddResult(SearchResult result);
    std::vector<SearchResult> GetResults();
    void AddRootMove(RootMove root_move);
    std::vector<RootMove> GetRootMoves();

  private:
    Event GetEvent();
    std::vector<SearchResult> results_;
    std::vector<RootMove> root_moves_;
    std::atomic<depth_t> depth_;
    std::atomic<uint8_t> is_stopped_;
    std::atomic<uint8_t> detailed_results_enabled_;
    std::condition_variable event_;
    std::mutex lock_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_CONTROL_CONTROL_H
