#ifndef QUIRKY_SRC_CORE_MOVES_MAGIC_H
#define QUIRKY_SRC_CORE_MOVES_MAGIC_H

#include <array>

#include "magic_calc.h"
#include "move.h"

namespace q_core {

const std::array<bitboard_t, BOARD_SIZE * 2> PAWN_REVERSED_ATTACK_BITBOARD =
    GetPawnReversedAttackBitboard();
const std::array<bitboard_t, BOARD_SIZE> KNIGHT_ATTACK_BITBOARD = GetKnightAttackBitboard();
const std::array<bitboard_t, BOARD_SIZE> KING_ATTACK_BITBOARD = GetKingAttackBitboard();

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MAGIC_H
