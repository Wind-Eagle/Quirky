#ifndef QUIRKY_SRC_EVAL_EVAL_FEATURES_H
#define QUIRKY_SRC_EVAL_EVAL_FEATURES_H

#include <array>

#include "../core/board/board.h"
#include "../core/board/geometry.h"
#include "pawns.h"

namespace q_eval {

struct PawnContext {
    q_core::bitboard_t our_pawns;
    q_core::coord_t pawn_coord;
    q_core::Color color;
};

bool IsPawnIsolated(const PawnContext& context);
bool IsPawnDoubled(const PawnContext& context);

struct KingSafety {
    uint8_t pawn_shield_mask;
    uint8_t pawn_storm_mask;
    bool is_side_queenside;
};

inline constexpr uint8_t PAWN_SHIELD_PAWN_COUNT = 6;
inline constexpr uint8_t PAWN_STORM_PAWN_COUNT = 6;

template <q_core::Color c>
KingSafety GetKingSafety(const q_core::Board& board);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_FEATURES_H
