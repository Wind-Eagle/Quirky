#ifndef QUIRKY_SRC_CORE_MOVES_ATTACK_H
#define QUIRKY_SRC_CORE_MOVES_ATTACK_H

#include <array>

#include "../board/board.h"

namespace q_core {

using see_score_t = int16_t;

template <Color c>
bool IsCellAttacked(const Board& board, coord_t src);

template <Color c>
bool IsKingInCheck(const Board& board);

bool IsKingInCheck(const Board& board);

template <Color c>
bitboard_t GetPawnAttacks(bitboard_t pawns);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_ATTACK_H
