#include "stat.h"

namespace q_search {

uint64_t SearchStat::GetNodesCount() const { return total_nodes_; }

uint64_t SearchStat::GetNodesCount(uint16_t move) const { return nodes_by_pv_.at(move); }

void SearchStat::IncNodesCount() { total_nodes_++; }

void SearchStat::OnRootMove(q_core::Move move) {
    const uint16_t compressed_move = q_core::GetCompressedMove(move);
    if (pv_moves_.empty()) {
        nodes_by_pv_[compressed_move] = total_nodes_;
    } else {
        nodes_by_pv_[compressed_move] = total_nodes_ - nodes_by_pv_[pv_moves_.back()];
    }
    pv_moves_.push_back(compressed_move);
}

}  // namespace q_search
