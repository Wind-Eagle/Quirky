#include "repetition_table.h"

namespace q_search {

RepetitionTable::RepetitionTable(const uint8_t byte_size_log)
    : size_mask_((1ULL << (byte_size_log - ENTRY_SIZE_LOG)) - 1),
      data_(new q_core::hash_t[1ULL << (byte_size_log - ENTRY_SIZE_LOG)]) {
    for (size_t i = 0; i < (1ULL << (byte_size_log - ENTRY_SIZE_LOG)); i++) {
        data_[i] = ABSENT_VALUE;
    }
}

bool RepetitionTable::Insert(const q_core::hash_t hash) {
    uint64_t key = hash & size_mask_;
    for (uint64_t i = key;; i = ((i + 1) & size_mask_)) {
        if (data_[i] == ABSENT_VALUE) {
            data_[i] = hash;
            return true;
        }
        if (data_[i] == hash) {
            return false;
        }
    }
}

void RepetitionTable::Erase(const q_core::hash_t hash) {
    uint64_t key = hash & size_mask_;
    for (uint64_t i = key;; i = ((i + 1) & size_mask_)) {
        if (data_[i] == hash) {
            data_[i] = ABSENT_VALUE;
            break;
        }
    }
}

bool RepetitionTable::Has(const q_core::hash_t hash) const {
    uint64_t key = hash & size_mask_;
    for (uint64_t i = key;; i = ((i + 1) & size_mask_)) {
        if (data_[i] == ABSENT_VALUE) {
            return false;
        }
        if (data_[i] == hash) {
            return true;
        }
    }
    Q_UNREACHABLE();
}

}  // namespace q_search
