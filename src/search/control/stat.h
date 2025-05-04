#ifndef QUIRKY_SRC_SEARCH_CONTROL_STAT_H
#define QUIRKY_SRC_SEARCH_CONTROL_STAT_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "core/moves/move.h"

namespace q_search {

class SearchStat {
  public:
    uint64_t GetNodesCount() const;
    uint64_t GetNodesCount(uint16_t move) const;
    void IncNodesCount();
    void OnRootMove(q_core::Move move);

  private:
    std::unordered_map<uint16_t, uint64_t> nodes_by_pv_;
    std::vector<uint16_t> pv_moves_;
    uint64_t total_nodes_ = 0;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_CONTROL_STAT_H
