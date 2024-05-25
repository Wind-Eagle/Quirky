#include "transposition_table.h"

namespace q_search {

uint64_t GetKeyHash(const q_core::hash_t hash, const uint8_t size_log) {
    return hash & ((1ULL << size_log) - 1);
}

uint32_t GetValueHash(const q_core::hash_t hash) { return hash >> 32; }

uint16_t GetEntryImportance(const auto entry, uint16_t generation) {
    return entry.depth + generation * 2;
}

bool IsEntryBetter(const auto lhs, const auto rhs) {
    uint8_t generation_diff =
        static_cast<int>(std::numeric_limits<decltype(lhs.info.GetGeneration())>::max()) + 1 +
        lhs.info.GetGeneration() - rhs.info.GetGeneration();
    return GetEntryImportance(lhs, generation_diff) > GetEntryImportance(rhs, generation_diff);
}

TranspositionTable::TranspositionTable(const uint8_t byte_size_log)
    : size_log_(byte_size_log - CLUSTER_SIZE_LOG),
      data_(new TranspositionTable::Cluster[(1ULL << (byte_size_log - CLUSTER_SIZE_LOG))]) {}

void TranspositionTable::Store(TranspositionTable::Entry& old_entry, const q_core::hash_t hash,
                               const q_core::Move move, const q_eval::score_t score, const uint8_t depth,
                               const NodeType node_type, const bool is_pv) const {
    const auto value_hash = GetValueHash(hash);
    Entry new_entry{.hash_low = static_cast<uint16_t>(value_hash & ((1ULL << 16) - 1)),
                    .hash_high = static_cast<uint16_t>(value_hash >> 16),
                    .score = score,
                    .move = q_core::GetCompressedMove(move),
                    .depth = depth,
                    .info = EntryInfo(generation_, node_type, is_pv)};
    if (IsEntryBetter(new_entry, old_entry)) {
        old_entry = new_entry;
    }
}

TranspositionTable::Entry& TranspositionTable::GetEntry(const q_core::hash_t hash, bool& found) const {
    const auto key_hash = GetKeyHash(hash, size_log_);
    const auto value_hash = GetValueHash(hash);
    auto& entry = data_[key_hash];
    for (uint8_t i = 0; i < Cluster::CLUSTER_ENTRY_COUNT; i++) {
        const auto entry_hash = entry.data[i].hash_low + (entry.data[i].hash_high << 16);
        if (entry_hash == value_hash) {
            found = true;
            return entry.data[i];
        }
    }
    found = false;
    return entry.data[0];
}

void TranspositionTable::Prefetch(const q_core::hash_t hash) const {
    const auto key_hash = GetKeyHash(hash, size_log_);
    Q_PREFETCH(&data_[key_hash]);
}

void TranspositionTable::Clear() {
    data_ = nullptr;
    generation_ = 0;
}

void TranspositionTable::ClearAndResize(uint8_t new_byte_size_log) {
    data_.reset(new TranspositionTable::Cluster[(1ULL << (new_byte_size_log - CLUSTER_SIZE_LOG))]);
    generation_ = 0;
}

void TranspositionTable::NextPosition() { generation_ += EntryInfo::GENERATION_DELTA; }

void TranspositionTable::NextGame() { generation_ -= EntryInfo::GENERATION_DELTA; }

}  // namespace q_search
