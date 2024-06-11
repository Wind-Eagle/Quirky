#ifndef QUIRKY_SRC_SEARCH_POSITION_TRANSPOSITION_TABLE_H
#define QUIRKY_SRC_SEARCH_POSITION_TRANSPOSITION_TABLE_H

#include <memory>

#include "../../core/board/types.h"
#include "../../core/moves/move.h"
#include "../../eval/score.h"

namespace q_search {

class TranspositionTable {
  public:
    enum class NodeType : uint8_t { Invalid = 0, ExactValue = 1, LowerBound = 2, UpperBound = 3 };
    struct EntryInfo {
        EntryInfo() {}
        EntryInfo(uint8_t generation, NodeType type, bool is_pv) {
            data_ = (generation << 3) + (static_cast<uint8_t>(type) << 1) + (is_pv ? 1 : 0);
        }
        uint8_t GetGeneration() const { return data_ >> GENERATION_BIT_COUNT; }
        NodeType GetNodeType() const {
            return static_cast<NodeType>((data_ >> 1) & ((1 << NODE_TYPE_BIT_COUNT) - 1));
        }
        bool IsPV() const { return data_ & 1; }

      private:
        uint8_t data_ = 0;
        static constexpr uint8_t NODE_TYPE_BIT_COUNT = 2;

      public:
        static constexpr uint8_t GENERATION_BIT_COUNT = NODE_TYPE_BIT_COUNT + 1;
    };
    struct Entry {
        // actually, we can store 16 bit of hash, but it requires a number of experiments
        // storing 32-bit hash as two numbers to get proper struct alignment
        uint16_t hash_low;
        uint16_t hash_high;
        q_eval::score_t score;
        q_core::compressed_move_t move;
        uint8_t depth = 0;
        EntryInfo info;
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
    static constexpr uint8_t CLUSTER_SIZE_LOG = q_util::GetHighestBit(sizeof(Cluster));

    void NextPosition();
    void NextGame();

    void Clear();
    void ClearAndResize(uint8_t new_byte_size_log);

    explicit TranspositionTable(uint8_t byte_size_log);

    void Store(TranspositionTable::Entry& old_entry, q_core::hash_t hash, q_core::Move move, q_eval::score_t score, uint8_t depth,
               NodeType node_type, bool is_pv) const;
    Entry& GetEntry(q_core::hash_t hash, bool& found) const;
    void Prefetch(q_core::hash_t hash) const;

  private:
    std::unique_ptr<Cluster[]> data_;
    uint8_t generation_;
    uint8_t size_log_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_TRANSPOSITION_TABLE_H
