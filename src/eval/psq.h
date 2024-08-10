#ifndef QUIRKY_SRC_EVAL_PSQ_H
#define QUIRKY_SRC_EVAL_PSQ_H

#include <array>

#include "../core/board/board.h"
#include "../core/util.h"
#include "score.h"

namespace q_eval {

inline constexpr uint16_t PSQ_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_CELLS;

inline constexpr uint16_t GetPSQIndex(const q_core::cell_t cell, q_core::coord_t src) {
    return cell * q_core::BOARD_SIZE + src;
}

// Weights inherited from here
// https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
// https://www.chessprogramming.org/Texel's_Tuning_Method

inline constexpr std::array<ScorePair, q_core::NUMBER_OF_PIECES> PIECE_COST = {
    ScorePair(82, 94),   ScorePair(337, 281),  ScorePair(365, 297),
    ScorePair(477, 512), ScorePair(1025, 936), ScorePair(0, 0)};

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

inline constexpr std::array<ScorePair, PSQ_SIZE> PSQ =
    GetPSQ({ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-35, 13),   ScorePair(-1, 8),    ScorePair(-20, 8),   ScorePair(-23, 10),
            ScorePair(-15, 13),   ScorePair(24, 0),    ScorePair(38, 2),    ScorePair(-22, -7),
            ScorePair(-26, 4),    ScorePair(-4, 7),    ScorePair(-4, -6),   ScorePair(-10, 1),
            ScorePair(3, 0),      ScorePair(3, -5),    ScorePair(33, -1),   ScorePair(-12, -8),
            ScorePair(-27, 13),   ScorePair(-2, 9),    ScorePair(-5, -3),   ScorePair(12, -7),
            ScorePair(17, -7),    ScorePair(6, -8),    ScorePair(10, 3),    ScorePair(-25, -1),
            ScorePair(-14, 32),   ScorePair(13, 24),   ScorePair(6, 13),    ScorePair(21, 5),
            ScorePair(23, -2),    ScorePair(12, 4),    ScorePair(17, 17),   ScorePair(-23, 17),
            ScorePair(-6, 94),    ScorePair(7, 100),   ScorePair(26, 85),   ScorePair(31, 67),
            ScorePair(65, 56),    ScorePair(56, 53),   ScorePair(25, 82),   ScorePair(-20, 84),
            ScorePair(98, 178),   ScorePair(134, 173), ScorePair(61, 158),  ScorePair(95, 134),
            ScorePair(68, 147),   ScorePair(126, 132), ScorePair(34, 165),  ScorePair(-11, 187),
            ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-105, -29), ScorePair(-21, -51), ScorePair(-58, -23), ScorePair(-33, -15),
            ScorePair(-17, -22),  ScorePair(-28, -18), ScorePair(-19, -50), ScorePair(-23, -64),
            ScorePair(-29, -42),  ScorePair(-53, -20), ScorePair(-12, -10), ScorePair(-3, -5),
            ScorePair(-1, -2),    ScorePair(18, -20),  ScorePair(-14, -23), ScorePair(-19, -44),
            ScorePair(-23, -23),  ScorePair(-9, -3),   ScorePair(12, -1),   ScorePair(10, 15),
            ScorePair(19, 10),    ScorePair(17, -3),   ScorePair(25, -20),  ScorePair(-16, -22),
            ScorePair(-13, -18),  ScorePair(4, -6),    ScorePair(16, 16),   ScorePair(13, 25),
            ScorePair(28, 16),    ScorePair(19, 17),   ScorePair(21, 4),    ScorePair(-8, -18),
            ScorePair(-9, -17),   ScorePair(17, 3),    ScorePair(19, 22),   ScorePair(53, 22),
            ScorePair(37, 22),    ScorePair(69, 11),   ScorePair(18, 8),    ScorePair(22, -18),
            ScorePair(-47, -24),  ScorePair(60, -20),  ScorePair(37, 10),   ScorePair(65, 9),
            ScorePair(84, -1),    ScorePair(129, -9),  ScorePair(73, -19),  ScorePair(44, -41),
            ScorePair(-73, -25),  ScorePair(-41, -8),  ScorePair(72, -25),  ScorePair(36, -2),
            ScorePair(23, -9),    ScorePair(62, -25),  ScorePair(7, -24),   ScorePair(-17, -52),
            ScorePair(-167, -58), ScorePair(-89, -38), ScorePair(-34, -13), ScorePair(-49, -28),
            ScorePair(61, -31),   ScorePair(-97, -27), ScorePair(-15, -63), ScorePair(-107, -99),
            ScorePair(-33, -23),  ScorePair(-3, -9),   ScorePair(-14, -23), ScorePair(-21, -5),
            ScorePair(-13, -9),   ScorePair(-12, -16), ScorePair(-39, -5),  ScorePair(-21, -17),
            ScorePair(4, -14),    ScorePair(15, -18),  ScorePair(16, -7),   ScorePair(0, -1),
            ScorePair(7, 4),      ScorePair(21, -9),   ScorePair(33, -15),  ScorePair(1, -27),
            ScorePair(0, -12),    ScorePair(15, -3),   ScorePair(15, 8),    ScorePair(15, 10),
            ScorePair(14, 13),    ScorePair(27, 3),    ScorePair(18, -7),   ScorePair(10, -15),
            ScorePair(-6, -6),    ScorePair(13, 3),    ScorePair(13, 13),   ScorePair(26, 19),
            ScorePair(34, 7),     ScorePair(12, 10),   ScorePair(10, -3),   ScorePair(4, -9),
            ScorePair(-4, -3),    ScorePair(5, 9),     ScorePair(19, 12),   ScorePair(50, 9),
            ScorePair(37, 14),    ScorePair(37, 10),   ScorePair(7, 3),     ScorePair(-2, 2),
            ScorePair(-16, 2),    ScorePair(37, -8),   ScorePair(43, 0),    ScorePair(40, -1),
            ScorePair(35, -2),    ScorePair(50, 6),    ScorePair(37, 0),    ScorePair(-2, 4),
            ScorePair(-26, -8),   ScorePair(16, -4),   ScorePair(-18, 7),   ScorePair(-13, -12),
            ScorePair(30, -3),    ScorePair(59, -13),  ScorePair(18, -4),   ScorePair(-47, -14),
            ScorePair(-29, -14),  ScorePair(4, -21),   ScorePair(-82, -11), ScorePair(-37, -8),
            ScorePair(-25, -7),   ScorePair(-42, -9),  ScorePair(7, -17),   ScorePair(-8, -24),
            ScorePair(-19, -9),   ScorePair(-13, 2),   ScorePair(1, 3),     ScorePair(17, -1),
            ScorePair(16, -5),    ScorePair(7, -13),   ScorePair(-37, 4),   ScorePair(-26, -20),
            ScorePair(-44, -6),   ScorePair(-16, -6),  ScorePair(-20, 0),   ScorePair(-9, 2),
            ScorePair(-1, -9),    ScorePair(11, -9),   ScorePair(-6, -11),  ScorePair(-71, -3),
            ScorePair(-45, -4),   ScorePair(-25, 0),   ScorePair(-16, -5),  ScorePair(-17, -1),
            ScorePair(3, -7),     ScorePair(0, -12),   ScorePair(-5, -8),   ScorePair(-33, -16),
            ScorePair(-36, 3),    ScorePair(-26, 5),   ScorePair(-12, 8),   ScorePair(-1, 4),
            ScorePair(9, -5),     ScorePair(-7, -6),   ScorePair(6, -8),    ScorePair(-23, -11),
            ScorePair(-24, 4),    ScorePair(-11, 3),   ScorePair(7, 13),    ScorePair(26, 1),
            ScorePair(24, 2),     ScorePair(35, 1),    ScorePair(-8, -1),   ScorePair(-20, 2),
            ScorePair(-5, 7),     ScorePair(19, 7),    ScorePair(26, 7),    ScorePair(36, 5),
            ScorePair(17, 4),     ScorePair(45, -3),   ScorePair(61, -5),   ScorePair(16, -3),
            ScorePair(27, 11),    ScorePair(32, 13),   ScorePair(58, 13),   ScorePair(62, 11),
            ScorePair(80, -3),    ScorePair(67, 3),    ScorePair(26, 8),    ScorePair(44, 3),
            ScorePair(32, 13),    ScorePair(42, 10),   ScorePair(32, 18),   ScorePair(51, 15),
            ScorePair(63, 12),    ScorePair(9, 12),    ScorePair(31, 8),    ScorePair(43, 5),
            ScorePair(-1, -33),   ScorePair(-18, -28), ScorePair(-9, -22),  ScorePair(10, -43),
            ScorePair(-15, -5),   ScorePair(-25, -32), ScorePair(-31, -20), ScorePair(-50, -41),
            ScorePair(-35, -22),  ScorePair(-8, -23),  ScorePair(11, -30),  ScorePair(2, -16),
            ScorePair(8, -16),    ScorePair(15, -23),  ScorePair(-3, -36),  ScorePair(1, -32),
            ScorePair(-14, -16),  ScorePair(2, -27),   ScorePair(-11, 15),  ScorePair(-2, 6),
            ScorePair(-5, 9),     ScorePair(2, 17),    ScorePair(14, 10),   ScorePair(5, 5),
            ScorePair(-9, -18),   ScorePair(-26, 28),  ScorePair(-9, 19),   ScorePair(-10, 47),
            ScorePair(-2, 31),    ScorePair(-4, 34),   ScorePair(3, 39),    ScorePair(-3, 23),
            ScorePair(-27, 3),    ScorePair(-27, 22),  ScorePair(-16, 24),  ScorePair(-16, 45),
            ScorePair(-1, 57),    ScorePair(17, 40),   ScorePair(-2, 57),   ScorePair(1, 36),
            ScorePair(-13, -20),  ScorePair(-17, 6),   ScorePair(7, 9),     ScorePair(8, 49),
            ScorePair(29, 47),    ScorePair(56, 35),   ScorePair(47, 19),   ScorePair(57, 9),
            ScorePair(-24, -17),  ScorePair(-39, 20),  ScorePair(-5, 32),   ScorePair(1, 41),
            ScorePair(-16, 58),   ScorePair(57, 25),   ScorePair(28, 30),   ScorePair(54, 0),
            ScorePair(-28, -9),   ScorePair(0, 22),    ScorePair(29, 22),   ScorePair(12, 27),
            ScorePair(59, 27),    ScorePair(44, 19),   ScorePair(43, 10),   ScorePair(45, 20),
            ScorePair(-15, -53),  ScorePair(36, -34),  ScorePair(12, -21),  ScorePair(-54, -11),
            ScorePair(8, -28),    ScorePair(-28, -14), ScorePair(24, -24),  ScorePair(14, -43),
            ScorePair(1, -27),    ScorePair(7, -11),   ScorePair(-8, 4),    ScorePair(-64, 13),
            ScorePair(-43, 14),   ScorePair(-16, 4),   ScorePair(9, -5),    ScorePair(8, -17),
            ScorePair(-14, -19),  ScorePair(-14, -3),  ScorePair(-22, 11),  ScorePair(-46, 21),
            ScorePair(-44, 23),   ScorePair(-30, 16),  ScorePair(-15, 7),   ScorePair(-27, -9),
            ScorePair(-49, -18),  ScorePair(-1, -4),   ScorePair(-27, 21),  ScorePair(-39, 24),
            ScorePair(-46, 27),   ScorePair(-44, 23),  ScorePair(-33, 9),   ScorePair(-51, -11),
            ScorePair(-17, -8),   ScorePair(-20, 22),  ScorePair(-12, 24),  ScorePair(-27, 27),
            ScorePair(-30, 26),   ScorePair(-25, 33),  ScorePair(-14, 26),  ScorePair(-36, 3),
            ScorePair(-9, 10),    ScorePair(24, 17),   ScorePair(2, 23),    ScorePair(-16, 15),
            ScorePair(-20, 20),   ScorePair(6, 45),    ScorePair(22, 44),   ScorePair(-22, 13),
            ScorePair(29, -12),   ScorePair(-1, 17),   ScorePair(-20, 14),  ScorePair(-7, 17),
            ScorePair(-8, 17),    ScorePair(-4, 38),   ScorePair(-38, 23),  ScorePair(-29, 11),
            ScorePair(-65, -74),  ScorePair(23, -35),  ScorePair(16, -18),  ScorePair(-15, -18),
            ScorePair(-56, -11),  ScorePair(-34, 15),  ScorePair(2, 4),     ScorePair(13, -17)});

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
