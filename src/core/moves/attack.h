#ifndef QUIRKY_SRC_CORE_MOVES_ATTACK_H
#define QUIRKY_SRC_CORE_MOVES_ATTACK_H

#include <array>

#include "../board/board.h"
#include "core/board/types.h"
#include "move.h"

namespace q_core {

template <Color c>
bool IsCellAttacked(const Board& board, coord_t src);

template <Color c>
bitboard_t GetCellAttackers(const Board& board, coord_t src);

template <Color c>
bool IsKingInCheck(const Board& board);
bool IsKingInCheck(const Board& board);

bool IsSEENotNegative(const Board& board, Move move, int16_t min_score,
                      const std::array<int16_t, NUMBER_OF_CELLS>& see_cells_cost);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_ATTACK_H
