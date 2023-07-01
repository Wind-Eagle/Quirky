#ifndef QUIRKY_SRC_CORE_MOVES_ATTACK_H
#define QUIRKY_SRC_CORE_MOVES_ATTACK_H

#include <array>

#include "../board/board.h"

namespace q_core {

using see_score_t = int16_t;

template <Color c>
bool IsCellAttacked(const Board& board, coord_t dst);

see_score_t GetSEE(const Board& board, coord_t dst,
                   const std::array<see_score_t, NUMBER_OF_CELLS>& cells_cost);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_ATTACK_H