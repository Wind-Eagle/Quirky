#include "stat.h"

namespace q_search {

uint64_t SearchStat::GetNodesCount() const { return nodes_; }

void SearchStat::IncNodesCount() { nodes_++; }

}  // namespace q_search
