#ifndef QUIRKY_SRC_SEARCH_CONTROL_CONTROL_H
#define QUIRKY_SRC_SEARCH_CONTROL_CONTROL_H

#include <condition_variable>
#include <vector>

#include "stat.h"
#include "../position/position.h"
#include "../../core/moves/move.h"
#include "../../eval/score.h"

namespace q_search {

enum SearchResultBoundType {
    Exact,
    Lower,
    Upper
};

struct SearchResult {
    SearchResultBoundType bound_type;
    q_eval::score_t score;
    q_core::Move best_move;
    depth_t depth;
    std::vector<q_core::Move> pv;
};

class SearchControl {
    public:
        enum class Event {
            Timeout,
            Stop,
            NewResult
        };
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
        bool IsStopped() const;
        depth_t GetDepth() const;
        bool FinishDepth(depth_t depth);
        void AddResult(SearchResult result);
        std::vector<SearchResult> GetResults();
    private:
        Event GetEvent();
        std::vector<SearchResult> results_;
        std::atomic<depth_t> depth_;
        std::atomic<uint8_t> is_stopped_;
        std::condition_variable event_;
        std::mutex lock_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_CONTROL_CONTROL_H
