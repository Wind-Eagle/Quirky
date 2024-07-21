#ifndef QUIRKY_SRC_EVAL_EVAL_FEATURES_H
#define QUIRKY_SRC_EVAL_EVAL_FEATURES_H

#include <array>

#include "../core/board/board.h"
#include "../core/board/geometry.h"
#include "feature.h"
#include "score.h"

namespace q_eval {

struct PawnContext {
    q_core::bitboard_t our_pawns;
    q_core::coord_t pawn_coord;
    q_core::Color color;
};

bool IsPawnIsolated(const PawnContext& context);
bool IsPawnDoubled(const PawnContext& context);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_FEATURES_H
