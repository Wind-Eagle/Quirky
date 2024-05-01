#ifndef QUIRKY_SRC_SEARCH_POSITION_TRANSPOSITION_TABLE_H
#define QUIRKY_SRC_SEARCH_POSITION_TRANSPOSITION_TABLE_H

#include "../../core/board/types.h"
#include "../../core/moves/move.h"
#include "../../eval/score.h"

#include <memory>

namespace q_search {

template <size_t SIZE_LOG>
class TranspositionTable {
  public:
    enum class NodeType : uint8_t {
        ExactValue = 0,
        LowerBound = 1,
        UpperBound = 2,
        Invalid = 3
    };
    struct EntryInfo {
        uint8_t data;
        uint8_t GetGeneration() const {
            return data >> (NODE_TYPE_BIT_COUNT + 1);
        }
        NodeType GetNodeType() const {
            return static_cast<NodeType>((data >> 1) & ((1 << NODE_TYPE_BIT_COUNT) - 1));
        }
        bool IsPV() const {
            return data & 1;
        }
        private:
            static constexpr uint8_t NODE_TYPE_BIT_COUNT = 2;
    };
    struct Entry {
        // actually, we can store 16 bit of hash, but it requires a number of experiments
        // storing 32-bit hash as two numbers to get proper struct alignment
        uint16_t hash_left;
        uint16_t hash_right;
        q_eval::score_t score;
        q_core::compressed_move_t move;
        uint8_t depth;
        EntryInfo flags;
    };

    struct Cluster {
        public:
            static constexpr uint8_t CLUSTER_ENTRY_COUNT = 3;
            std::array<Entry, CLUSTER_ENTRY_COUNT> data;
        private:
            static constexpr uint8_t PADDING_SIZE = 2;
            std::array<char, PADDING_SIZE> padding_;
   };

   Q_STATIC_ASSERT(q_util::GetBitCount(sizeof(Cluster)) == 1);

   TranspositionTable() = default;
   ~TranspositionTable() = default;
   
   void NextPosition();
   void NextGame();

   void Clear();

   void Store(q_core::hash_t hash);
   Entry Load(q_core::hash_t hash);
   void Prefetch(q_core::hash_t hash);

  private:
    static constexpr size_t SIZE = (1ULL << SIZE_LOG) / sizeof(Cluster);
    std::unique_ptr<std::array<Cluster, SIZE>> data_;
    uint8_t generation_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_TRANSPOSITION_TABLE_H
