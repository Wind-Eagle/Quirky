#ifndef QUIRKY_SRC_EVAL_EVAL_FEATURES_H
#define QUIRKY_SRC_EVAL_EVAL_FEATURES_H

#include <array>

#include "../core/board/board.h"
#include "../core/board/geometry.h"
#include "feature.h"
#include "score.h"

namespace q_eval {

extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> WHITE_PAWN_FRONTSPAN_BITBOARD;
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> BLACK_PAWN_FRONTSPAN_BITBOARD;
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIDE> PAWN_NEIGHBOURS_BITBOARD;

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_FEATURES_H
