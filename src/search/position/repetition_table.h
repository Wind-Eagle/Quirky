#ifndef QUIRKY_SRC_SEARCH_POSITION_REPETITION_TABLE_H
#define QUIRKY_SRC_SEARCH_POSITION_REPETITION_TABLE_H

#include <memory>

#include "core/board/types.h"
#include "util/bit.h"

namespace q_search {

class RepetitionTable {
  public:
    explicit RepetitionTable(uint8_t byte_size_log);

    bool Insert(q_core::hash_t hash);
    void Erase(q_core::hash_t hash);
    bool Has(q_core::hash_t hash) const;

  private:
    static constexpr uint8_t ENTRY_SIZE_LOG = q_util::GetHighestBit(sizeof(q_core::hash_t));
    static constexpr q_core::hash_t ABSENT_VALUE = 0;
    std::unique_ptr<q_core::hash_t[]> data_;
    size_t size_mask_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_REPETITION_TABLE_H
