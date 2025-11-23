#ifndef QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H

#include <thread>

#include "search/control/control.h"
#include "search/control/time.h"
#include "search/position/position.h"
#include "search/position/transposition_table.h"

namespace q_search {

class SearchLauncher {
  public:
    ~SearchLauncher();
    void Start(const Position& start_position, const std::vector<q_core::Move>& moves,
               time_control_t time_control, depth_t max_depth);
    void Stop();
    void Join();
    void NewGame();
    void ChangeTTSize(size_t new_tt_size_mb);
    void ChangePVCount(size_t new_pv_count);

  private:
    void StartMainThread(const Position& start_position, const std::vector<q_core::Move>& moves,
                         time_control_t time_control, depth_t max_depth);
    static constexpr uint8_t TT_DEFAULT_BYTE_SIZE_LOG = 23;
    std::thread thread_;
    q_search::TranspositionTable tt_{TT_DEFAULT_BYTE_SIZE_LOG};
    SearchControl control_;
    size_t pv_count_ = 1;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H
