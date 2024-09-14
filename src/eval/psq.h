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

EVAL_CONSTS_TYPE std::array<ScorePair, q_core::NUMBER_OF_PIECES> PIECE_COST = {
    ScorePair(89, 110),  ScorePair(403, 229),  ScorePair(449, 256),
    ScorePair(561, 472), ScorePair(906, 1013), ScorePair(0, 0)};

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
    GetPSQ({ScorePair(0, 0),     ScorePair(0, 0),      ScorePair(0, 0),      ScorePair(0, 0),
            ScorePair(0, 0),     ScorePair(0, 0),      ScorePair(0, 0),      ScorePair(0, 0),
            ScorePair(-44, 0),   ScorePair(-8, 3),     ScorePair(-18, 11),   ScorePair(-18, 10),
            ScorePair(-5, 6),    ScorePair(32, -2),    ScorePair(52, -14),   ScorePair(-6, -20),
            ScorePair(-42, -7),  ScorePair(-21, 3),    ScorePair(0, -9),     ScorePair(-7, 1),
            ScorePair(10, 0),    ScorePair(-6, -3),    ScorePair(29, -11),   ScorePair(-8, -21),
            ScorePair(-50, 0),   ScorePair(-15, 8),    ScorePair(-9, -12),   ScorePair(18, -12),
            ScorePair(11, -10),  ScorePair(2, -12),    ScorePair(-5, -4),    ScorePair(-47, -12),
            ScorePair(-24, 11),  ScorePair(-6, 12),    ScorePair(0, 0),      ScorePair(14, -12),
            ScorePair(25, -16),  ScorePair(11, -11),   ScorePair(1, 1),      ScorePair(-24, -8),
            ScorePair(-27, 28),  ScorePair(-16, 35),   ScorePair(20, 19),    ScorePair(22, 15),
            ScorePair(42, 0),    ScorePair(87, -19),   ScorePair(6, 6),      ScorePair(-8, -1),
            ScorePair(37, 207),  ScorePair(25, 214),   ScorePair(21, 193),   ScorePair(59, 152),
            ScorePair(63, 143),  ScorePair(49, 162),   ScorePair(0, 207),    ScorePair(-7, 189),
            ScorePair(0, 0),     ScorePair(0, 0),      ScorePair(0, 0),      ScorePair(0, 0),
            ScorePair(0, 0),     ScorePair(0, 0),      ScorePair(0, 0),      ScorePair(0, 0),
            ScorePair(-77, -35), ScorePair(-24, -34),  ScorePair(-62, 0),    ScorePair(-39, 4),
            ScorePair(-38, -5),  ScorePair(-17, 1),    ScorePair(-30, -34),  ScorePair(-93, -24),
            ScorePair(-27, -19), ScorePair(-35, -1),   ScorePair(-3, -3),    ScorePair(1, 13),
            ScorePair(3, 7),     ScorePair(5, 8),      ScorePair(-3, -9),    ScorePair(-10, -23),
            ScorePair(-22, -7),  ScorePair(-6, 10),    ScorePair(21, 6),     ScorePair(11, 32),
            ScorePair(24, 33),   ScorePair(12, 13),    ScorePair(25, -5),    ScorePair(-29, -16),
            ScorePair(-14, 4),   ScorePair(-2, 13),    ScorePair(17, 35),    ScorePair(14, 37),
            ScorePair(26, 41),   ScorePair(25, 24),    ScorePair(4, 9),      ScorePair(-26, 3),
            ScorePair(2, -1),    ScorePair(16, 26),    ScorePair(11, 34),    ScorePair(60, 41),
            ScorePair(24, 36),   ScorePair(51, 29),    ScorePair(5, 24),     ScorePair(21, -8),
            ScorePair(-1, -14),  ScorePair(11, 0),     ScorePair(24, 27),    ScorePair(69, 23),
            ScorePair(87, 0),    ScorePair(107, 5),    ScorePair(57, -6),    ScorePair(-11, -9),
            ScorePair(-90, -12), ScorePair(-46, 0),    ScorePair(69, -13),   ScorePair(17, 7),
            ScorePair(71, -16),  ScorePair(63, -15),   ScorePair(-15, -9),   ScorePair(-12, -44),
            ScorePair(-268, -9), ScorePair(-89, -39),  ScorePair(-73, -13),  ScorePair(-66, 3),
            ScorePair(4, -15),   ScorePair(-132, -12), ScorePair(-141, -20), ScorePair(-156, -76),
            ScorePair(-10, -28), ScorePair(-27, -9),   ScorePair(-9, -32),   ScorePair(-24, 0),
            ScorePair(-36, 0),   ScorePair(-28, -13),  ScorePair(-29, -24),  ScorePair(-18, -14),
            ScorePair(-29, -3),  ScorePair(25, -22),   ScorePair(4, -8),     ScorePair(-1, 5),
            ScorePair(1, 4),     ScorePair(19, -5),    ScorePair(37, -14),   ScorePair(7, -29),
            ScorePair(18, -17),  ScorePair(19, 1),     ScorePair(15, 9),     ScorePair(7, 13),
            ScorePair(12, 20),   ScorePair(16, 6),     ScorePair(9, -10),    ScorePair(14, -18),
            ScorePair(-19, -4),  ScorePair(-6, 5),     ScorePair(7, 16),     ScorePair(25, 10),
            ScorePair(31, 14),   ScorePair(8, 17),     ScorePair(3, -5),     ScorePair(-38, -3),
            ScorePair(-41, 11),  ScorePair(-8, 14),    ScorePair(7, 12),     ScorePair(22, 18),
            ScorePair(23, 20),   ScorePair(-7, 8),     ScorePair(3, 4),      ScorePair(-9, -8),
            ScorePair(-26, 1),   ScorePair(-6, 3),     ScorePair(44, 3),     ScorePair(12, 12),
            ScorePair(51, 0),    ScorePair(43, 10),    ScorePair(33, 0),     ScorePair(33, -10),
            ScorePair(-45, -9),  ScorePair(1, 5),      ScorePair(-26, 5),    ScorePair(-29, 0),
            ScorePair(-3, 1),    ScorePair(46, -14),   ScorePair(-14, 5),    ScorePair(12, -36),
            ScorePair(-48, -5),  ScorePair(-66, -15),  ScorePair(-62, -15),  ScorePair(-96, 6),
            ScorePair(-139, 9),  ScorePair(-58, -2),   ScorePair(-78, -10),  ScorePair(-111, 7),
            ScorePair(-23, -2),  ScorePair(-18, 3),    ScorePair(2, 0),      ScorePair(0, 8),
            ScorePair(8, -5),    ScorePair(-8, -7),    ScorePair(-31, -1),   ScorePair(-9, -41),
            ScorePair(-54, 6),   ScorePair(-27, -2),   ScorePair(-29, 10),   ScorePair(-22, 1),
            ScorePair(-15, -2),  ScorePair(-12, -3),   ScorePair(10, -15),   ScorePair(-56, -7),
            ScorePair(-49, -2),  ScorePair(-32, 0),    ScorePair(-9, -4),    ScorePair(-3, -4),
            ScorePair(-9, -1),   ScorePair(-12, -10),  ScorePair(12, -25),   ScorePair(-2, -21),
            ScorePair(-39, 8),   ScorePair(-46, 8),    ScorePair(-32, 14),   ScorePair(-19, 11),
            ScorePair(-9, 4),    ScorePair(-17, 0),    ScorePair(0, -9),     ScorePair(-6, -14),
            ScorePair(-38, 14),  ScorePair(-17, 5),    ScorePair(5, 8),      ScorePair(21, 0),
            ScorePair(0, 3),     ScorePair(15, 0),     ScorePair(31, -8),    ScorePair(31, -15),
            ScorePair(-3, 10),   ScorePair(-4, 10),    ScorePair(4, 10),     ScorePair(34, -1),
            ScorePair(53, -12),  ScorePair(52, -8),    ScorePair(71, -9),    ScorePair(44, -14),
            ScorePair(0, 12),    ScorePair(-7, 27),    ScorePair(33, 12),    ScorePair(59, 3),
            ScorePair(24, 4),    ScorePair(86, -5),    ScorePair(75, -7),    ScorePair(53, -12),
            ScorePair(32, 18),   ScorePair(29, 11),    ScorePair(38, 15),    ScorePair(59, 4),
            ScorePair(70, 0),    ScorePair(50, -2),    ScorePair(2, 1),      ScorePair(21, 4),
            ScorePair(6, -34),   ScorePair(-9, -48),   ScorePair(0, -39),    ScorePair(20, -77),
            ScorePair(-3, -24),  ScorePair(-25, -36),  ScorePair(-69, -39),  ScorePair(-11, -79),
            ScorePair(-16, -18), ScorePair(15, -24),   ScorePair(15, -28),   ScorePair(14, -11),
            ScorePair(20, -17),  ScorePair(35, -73),   ScorePair(37, -81),   ScorePair(8, -62),
            ScorePair(-8, -51),  ScorePair(14, -47),   ScorePair(5, 9),      ScorePair(8, -4),
            ScorePair(4, 15),    ScorePair(15, 0),     ScorePair(17, 7),     ScorePair(1, -31),
            ScorePair(-8, -17),  ScorePair(2, -6),     ScorePair(-6, 12),    ScorePair(0, 38),
            ScorePair(9, 26),    ScorePair(4, 6),      ScorePair(15, 5),     ScorePair(-4, -6),
            ScorePair(-18, -32), ScorePair(-12, 16),   ScorePair(-11, 12),   ScorePair(-6, 41),
            ScorePair(-3, 60),   ScorePair(4, 54),     ScorePair(-9, 26),    ScorePair(23, 0),
            ScorePair(-15, -37), ScorePair(-16, -14),  ScorePair(6, 24),     ScorePair(30, 12),
            ScorePair(40, 57),   ScorePair(77, -1),    ScorePair(116, -32),  ScorePair(75, -37),
            ScorePair(-21, -5),  ScorePair(-32, 9),    ScorePair(-32, 24),   ScorePair(-62, 73),
            ScorePair(-44, 49),  ScorePair(84, 38),    ScorePair(13, 34),    ScorePair(125, -94),
            ScorePair(-56, 13),  ScorePair(-85, 50),   ScorePair(-30, 29),   ScorePair(-29, 47),
            ScorePair(-22, 44),  ScorePair(-36, 52),   ScorePair(-103, 33),  ScorePair(-83, 51),
            ScorePair(-42, -32), ScorePair(34, -33),   ScorePair(1, -13),    ScorePair(-81, -2),
            ScorePair(0, -31),   ScorePair(-41, -6),   ScorePair(50, -40),   ScorePair(43, -67),
            ScorePair(40, -31),  ScorePair(-8, -4),    ScorePair(-30, 15),   ScorePair(-94, 34),
            ScorePair(-70, 31),  ScorePair(-34, 24),   ScorePair(32, 0),     ScorePair(35, -23),
            ScorePair(11, -33),  ScorePair(25, -8),    ScorePair(-62, 23),   ScorePair(-84, 36),
            ScorePair(-85, 43),  ScorePair(-54, 31),   ScorePair(-1, 11),    ScorePair(-30, 2),
            ScorePair(15, -39),  ScorePair(24, -8),    ScorePair(7, 14),     ScorePair(-79, 34),
            ScorePair(-37, 34),  ScorePair(-64, 39),   ScorePair(-35, 18),   ScorePair(-76, 4),
            ScorePair(10, -30),  ScorePair(22, -6),    ScorePair(58, 7),     ScorePair(3, 21),
            ScorePair(-12, 19),  ScorePair(-9, 29),    ScorePair(13, 19),    ScorePair(-24, 7),
            ScorePair(73, -30),  ScorePair(142, -6),   ScorePair(51, 7),     ScorePair(81, 2),
            ScorePair(26, 15),   ScorePair(83, 21),    ScorePair(91, 25),    ScorePair(-5, 11),
            ScorePair(47, -30),  ScorePair(108, -9),   ScorePair(102, -7),   ScorePair(55, -4),
            ScorePair(43, 6),    ScorePair(110, 9),    ScorePair(0, 25),     ScorePair(-18, 9),
            ScorePair(50, -65),  ScorePair(72, -33),   ScorePair(57, -26),   ScorePair(63, -23),
            ScorePair(72, -19),  ScorePair(98, -9),    ScorePair(40, -8),    ScorePair(27, -36)});

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
