#ifndef QUIRKY_SRC_CORE_MOVES_MAGIC_CALC_H
#define QUIRKY_SRC_CORE_MOVES_MAGIC_CALC_H

#include <array>

#include "../board/types.h"

namespace q_core {

std::array<bitboard_t, BOARD_SIZE * 2> GetPawnReversedAttackBitboard();
std::array<bitboard_t, BOARD_SIZE> GetKnightAttackBitboard();
std::array<bitboard_t, BOARD_SIZE> GetKingAttackBitboard();

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MAGIC_CALC_H
