#ifndef QUIRKY_SRC_EVAL_PAWNS_H
#define QUIRKY_SRC_EVAL_PAWNS_H

#include "score.h"

namespace q_eval {

struct PawnHashTableEntry {
    ScorePair score;
    uint8_t white_half_open_files_mask;
    uint8_t black_half_open_files_mask;
    uint16_t info;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_PAWNS_H
