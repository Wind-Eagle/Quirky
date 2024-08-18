#ifndef QUIRKY_SRC_EVAL_PSQ_H
#define QUIRKY_SRC_EVAL_PSQ_H

#include <array>

#include "../core/board/board.h"
#include "../core/util.h"
#include "score.h"

namespace q_eval {

#ifdef CHANGE_EVAL_CONSTS
#define EVAL_CONSTS_TYPE inline
#else
#define EVAL_CONSTS_TYPE inline constexpr
#endif

inline constexpr uint16_t PSQ_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_CELLS;

inline constexpr uint16_t GetPSQIndex(const q_core::cell_t cell, q_core::coord_t src) {
    return cell * q_core::BOARD_SIZE + src;
}

// Weights inherited from here
// https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
// https://www.chessprogramming.org/Texel's_Tuning_Method

EVAL_CONSTS_TYPE std::array<ScorePair, q_core::NUMBER_OF_PIECES> PIECE_COST = {
    ScorePair(100, 100),  ScorePair(300, 300),  ScorePair(300, 300),
    ScorePair(500, 500), ScorePair(900, 900), ScorePair(0, 0)};

inline constexpr std::array<ScorePair, PSQ_SIZE> GetPSQ(
    const std::array<ScorePair, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES>& psq) {
    std::array<ScorePair, PSQ_SIZE> res{};
    for (q_core::cell_t cell = 0; cell < q_core::NUMBER_OF_CELLS; cell++) {
        for (q_core::coord_t coord = 0; coord < q_core::BOARD_SIZE; coord++) {
            if (cell == q_core::EMPTY_CELL) {
                continue;
            }
            q_core::Piece piece = q_core::GetCellPiece(cell);
            const ScorePair& cur_value =
                psq[(static_cast<size_t>(piece) - 1) * q_core::BOARD_SIZE + coord];
            if (q_core::GetCellColor(cell) == q_core::Color::White) {
                res[GetPSQIndex(cell, coord)] =
                    cur_value + PIECE_COST[static_cast<uint8_t>(piece) - 1];
            } else {
                res[GetPSQIndex(cell, q_core::FlipCoord(coord))] =
                    (cur_value + PIECE_COST[static_cast<uint8_t>(piece) - 1]) * (-1);
            }
        }
    }
    return res;
}

EVAL_CONSTS_TYPE std::array<ScorePair, PSQ_SIZE> PSQ =
    GetPSQ({ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0),
            ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0), ScorePair(0, 0)});

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
