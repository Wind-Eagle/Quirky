#include "control.h"

namespace q_search {

void SearchControl::Stop() {
    uint8_t tmp = 0;
    if (!is_stopped_.compare_exchange_strong(tmp, 1, std::memory_order_release)) {
        return;
    }
    lock_.lock();
    lock_.unlock();
    event_.notify_all();
}

void SearchControl::EnableDetailedResults() {
    detailed_results_enabled_.store(1, std::memory_order_relaxed);
}

SearchControl::Event SearchControl::GetEvent() {
    if (!results_.empty()) {
        return Event::NewResult;
    }
    if (!root_moves_.empty()) {
        return Event::RootMove;
    }
    if (IsStopped()) {
        return Event::Stop;
    }
    return Event::Timeout;
}

void SearchControl::Reset() {
    depth_.store(1, std::memory_order_relaxed);
    is_stopped_.store(0, std::memory_order_relaxed);
    detailed_results_enabled_.store(0, std::memory_order_relaxed);
    results_.clear();
}

bool SearchControl::IsStopped() const { return is_stopped_.load(std::memory_order_acquire); }

depth_t SearchControl::GetDepth() const { return depth_.load(std::memory_order_acquire); }

bool SearchControl::FinishDepth(depth_t depth) {
    return depth_.compare_exchange_strong(depth, depth + 1, std::memory_order_acq_rel);
}

bool SearchControl::AreDetailedResultsEnabled() {
    return detailed_results_enabled_.load(std::memory_order_acquire);
}

void SearchControl::AddResult(SearchResult result) {
    std::unique_lock guard(lock_);
    results_.push_back(std::move(result));
    guard.unlock();
    event_.notify_all();
}

std::vector<SearchResult> SearchControl::GetResults() {
    std::unique_lock guard(lock_);
    auto res = std::move(results_);
    results_.clear();
    return res;
}

void SearchControl::AddRootMove(RootMove root_move) {
    if (!AreDetailedResultsEnabled()) {
        return;
    }
    std::unique_lock guard(lock_);
    root_moves_.push_back(root_move);
    guard.unlock();
    event_.notify_all();
}

std::vector<RootMove> SearchControl::GetRootMoves() {
    std::unique_lock guard(lock_);
    auto res = std::move(root_moves_);
    root_moves_.clear();
    return res;
}

}  // namespace q_search
