#ifndef QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H

#include <thread>

#include "../control/control.h"
#include "../control/time.h"
#include "../position/position.h"
#include "../position/transposition_table.h"

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

  private:
    void StartMainThread(const Position& start_position, const std::vector<q_core::Move>& moves,
                         time_control_t time_control, depth_t max_depth);
    static constexpr uint8_t TT_DEFAULT_BYTE_SIZE_LOG = 23;
    std::thread thread_;
    q_search::TranspositionTable tt_{TT_DEFAULT_BYTE_SIZE_LOG};
    SearchControl control_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H
