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
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> WHITE_PAWN_BACKWARD_SENTRY_BITBOARD;
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> BLACK_PAWN_BACKWARD_SENTRY_BITBOARD;
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> WHITE_PAWN_PASSED_BITBOARD;
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> BLACK_PAWN_PASSED_BITBOARD;
extern const std::array<q_core::bitboard_t, q_core::BOARD_SIZE> PAWN_CONNECTED_BITBOARD;
extern const std::array<uint8_t, 1 << q_core::BOARD_SIDE> PAWN_ISLANDS_COUNT;

extern const std::array<ScorePair, (1 << 6)> SHIELD_WEIGHTS;
extern const std::array<ScorePair, (1 << 6)> SHIELD_WEIGHTS_INVERTED;
extern const std::array<ScorePair, (1 << 6)> STORM_WEIGHTS;
extern const std::array<ScorePair, (1 << 6)> STORM_WEIGHTS_INVERTED;

inline constexpr q_core::bitboard_t WHITE_KING_SHIELDED_BITBOARD =
    q_core::RANK_BITBOARD[0] | q_core::RANK_BITBOARD[1];
inline constexpr q_core::bitboard_t BLACK_KING_SHIELDED_BITBOARD =
    q_core::RANK_BITBOARD[q_core::BOARD_SIDE - 1] |
    q_core::RANK_BITBOARD[q_core::BOARD_SIDE - 2];

uint8_t GetLInftyDistance(q_core::coord_t src, q_core::coord_t dst);

template <q_core::Color c>
bool IsPawnSurelyUnstoppable(const q_core::Board& board, q_core::coord_t pawn_coord);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_FEATURES_H
