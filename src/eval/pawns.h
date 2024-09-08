#ifndef QUIRKY_SRC_EVAL_PAWNS_H
#define QUIRKY_SRC_EVAL_PAWNS_H

#include "score.h"

#include <memory>

namespace q_eval {

struct PawnHashTableEntry {
    ScorePair score;
    uint32_t hash_part = 0;
};

inline bool IsPawnHashTableEntryValid(const PawnHashTableEntry entry) {
    return entry.hash_part > 0;
}

class PawnCache {
    public:
        PawnCache();
        PawnHashTableEntry Add(uint64_t hash, ScorePair score);
        bool Get(uint64_t hash, PawnHashTableEntry& entry) const;
    private:
        uint16_t GetKeyFromHash(uint64_t hash) const;
        std::unique_ptr<PawnHashTableEntry[]> data_;
        static constexpr uint8_t SIZE_LOG = 16;
};

PawnCache& GetPawnCacheRef();

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_PAWNS_H
