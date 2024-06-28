#ifndef QUIRKY_SRC_SEARCH_CONTROL_STAT_H
#define QUIRKY_SRC_SEARCH_CONTROL_STAT_H

#include <cstdint>

namespace q_search {

class SearchStat {
  public:
    uint64_t GetNodesCount() const;
    void IncNodesCount();

  private:
    uint64_t nodes_ = 0;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_CONTROL_STAT_H
