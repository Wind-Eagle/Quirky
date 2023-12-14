#ifndef QUIRKY_SRC_EVAL_PSQ_H
#define QUIRKY_SRC_EVAL_PSQ_H

#include <array>

#include "score.h"
#include "../core/board/board.h"

namespace q_eval {

inline constexpr uint16_t PSQ_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_CELLS;

inline constexpr std::array<ScorePair, PSQ_SIZE> PSQ = {};

Q_STATIC_ASSERT([](){
    for (size_t i = 0; i < q_core::BOARD_SIZE; i++) {
        if (PSQ[i] != ScorePair(0, 0)) {
            return false;
        }
    }
    return true;
});

inline constexpr uint16_t GetPSQIndex(const q_core::cell_t cell, q_core::coord_t src) {
    return cell * q_core::BOARD_SIZE + src;
}

inline constexpr q_core::coord_t GetCoordFromPSQIndex(uint16_t index) {
    return index % q_core::BOARD_SIZE;
}

inline constexpr q_core::cell_t GetCellFromPSQIndex(uint16_t index) {
    return index / q_core::BOARD_SIZE;
}

inline constexpr ScorePair GetPSQValue(const q_core::cell_t cell, q_core::coord_t src) {
    return PSQ[GetPSQIndex(cell, src)];
}

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_PSQ_H
