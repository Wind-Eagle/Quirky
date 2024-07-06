#ifndef QUIRKY_SRC_EVAL_PAWNS_H
#define QUIRKY_SRC_EVAL_PAWNS_H

#include "score.h"

namespace q_eval {

struct PawnHashTableEntry {
    ScorePair score;
    uint8_t white_open_files_mask;
    uint8_t black_open_files_mask;
    uint16_t info;
};

inline constexpr PawnHashTableEntry ConstructPawnHashTableEntry(ScorePair score, uint8_t white_open_files_mask, uint8_t black_open_files_mask) {
    return PawnHashTableEntry{.score = score, .white_open_files_mask = white_open_files_mask, .black_open_files_mask = black_open_files_mask, .info = 0};
}

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_PAWNS_H
