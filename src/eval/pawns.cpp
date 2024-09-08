#include "pawns.h"

namespace q_eval {

PawnCache::PawnCache() : data_(new PawnHashTableEntry[(1ULL << SIZE_LOG)]) {}

uint16_t PawnCache::GetKeyFromHash(const uint64_t hash) const {
    return hash >> (64 - SIZE_LOG);
}

uint32_t GetHashPart(const uint64_t hash) {
    return static_cast<uint32_t>(hash);
}

PawnHashTableEntry PawnCache::Add(const uint64_t hash, const ScorePair score) {
    PawnHashTableEntry entry = PawnHashTableEntry{.score = score, .hash_part = GetHashPart(hash)};
    const uint16_t key = GetKeyFromHash(hash);
    data_[key] = entry;
    return entry;
}

bool PawnCache::PawnCache::Get(const uint64_t hash, PawnHashTableEntry& entry) const {
    const uint16_t key = GetKeyFromHash(hash);
    if (!IsPawnHashTableEntryValid(data_[key]) || data_[key].hash_part != GetHashPart(hash)) {
        return false;
    }
    entry = data_[key];
    return true;
}

PawnCache& GetPawnCacheRef() {
    static PawnCache pawn_cache{};
    return pawn_cache;
}

}  // namespace q_eval