#ifndef QUIRKY_SRC_EVAL_PSQ_H
#define QUIRKY_SRC_EVAL_PSQ_H

#include <array>

#include "score.h"
#include "../core/board/board.h"

namespace q_eval {

inline constexpr uint16_t GetPSQIndex(const q_core::cell_t cell, q_core::coord_t src) {
    return cell * q_core::BOARD_SIZE + src;
}

inline constexpr uint16_t PSQ_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_CELLS;

inline constexpr std::array<std::array<ScorePair, q_core::BOARD_SIZE>, q_core::NUMBER_OF_CELLS> PSQ = {};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_PSQ_H
