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
    GetPSQ({ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),
ScorePair(-38, -6),ScorePair(-3, 10),ScorePair(-15, 0),ScorePair(-18, 1),ScorePair(-4, -6),ScorePair(28, -13),ScorePair(48, -5),ScorePair(-5, -32),
ScorePair(-35, -16),ScorePair(-17, 8),ScorePair(0, -21),ScorePair(-6, -7),ScorePair(9, -12),ScorePair(-5, -17),ScorePair(29, -4),ScorePair(-7, -31),
ScorePair(-42, -11),ScorePair(-13, 10),ScorePair(-8, -19),ScorePair(16, -23),ScorePair(8, -20),ScorePair(1, -23),ScorePair(-5, -1),ScorePair(-42, -25),
ScorePair(-23, 11),ScorePair(-3, 28),ScorePair(0, 0),ScorePair(13, -7),ScorePair(24, -16),ScorePair(9, -11),ScorePair(0, 14),ScorePair(-23, -8),
ScorePair(-32, 80),ScorePair(-7, 102),ScorePair(14, 77),ScorePair(11, 59),ScorePair(29, 45),ScorePair(60, 35),ScorePair(3, 80),ScorePair(-21, 62),
ScorePair(41, 147),ScorePair(30, 176),ScorePair(23, 148),ScorePair(58, 120),ScorePair(63, 111),ScorePair(46, 121),ScorePair(6, 171),ScorePair(-2, 132),
ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),
ScorePair(-71, -33),ScorePair(-21, -31),ScorePair(-58, -2),ScorePair(-35, 5),ScorePair(-33, -1),ScorePair(-12, 5),ScorePair(-27, -29),ScorePair(-86, -22),
ScorePair(-23, -18),ScorePair(-32, -2),ScorePair(-2, -2),ScorePair(2, 13),ScorePair(3, 10),ScorePair(5, 7),ScorePair(-4, -11),ScorePair(-8, -19),
ScorePair(-21, -6),ScorePair(-4, 13),ScorePair(20, 7),ScorePair(14, 31),ScorePair(24, 28),ScorePair(14, 11),ScorePair(24, -6),ScorePair(-27, -14),
ScorePair(-13, 1),ScorePair(-2, 10),ScorePair(16, 32),ScorePair(13, 36),ScorePair(23, 39),ScorePair(24, 23),ScorePair(3, 8),ScorePair(-26, 3),
ScorePair(2, 0),ScorePair(16, 25),ScorePair(13, 32),ScorePair(55, 39),ScorePair(23, 36),ScorePair(46, 28),ScorePair(4, 22),ScorePair(23, -3),
ScorePair(-6, -10),ScorePair(16, 4),ScorePair(24, 28),ScorePair(65, 24),ScorePair(80, 3),ScorePair(99, 7),ScorePair(52, -6),ScorePair(-8, -6),
ScorePair(-84, -11),ScorePair(-38, 0),ScorePair(65, -9),ScorePair(17, 8),ScorePair(65, -13),ScorePair(58, -14),ScorePair(-11, -9),ScorePair(-10, -38),
ScorePair(-238, -6),ScorePair(-81, -33),ScorePair(-64, -8),ScorePair(-50, 3),ScorePair(5, -11),ScorePair(-117, -11),ScorePair(-140, -17),ScorePair(-142, -69),
ScorePair(-11, -26),ScorePair(-27, -9),ScorePair(-11, -30),ScorePair(-21, 0),ScorePair(-33, -1),ScorePair(-25, -11),ScorePair(-26, -20),ScorePair(-18, -14),
ScorePair(-30, -8),ScorePair(23, -19),ScorePair(4, -7),ScorePair(0, 7),ScorePair(1, 8),ScorePair(18, -5),ScorePair(34, -11),ScorePair(4, -26),
ScorePair(13, -16),ScorePair(15, 0),ScorePair(12, 9),ScorePair(7, 14),ScorePair(10, 19),ScorePair(11, 5),ScorePair(9, -7),ScorePair(11, -15),
ScorePair(-22, -7),ScorePair(-6, 5),ScorePair(3, 14),ScorePair(24, 10),ScorePair(26, 13),ScorePair(6, 18),ScorePair(1, -7),ScorePair(-33, -6),
ScorePair(-38, 10),ScorePair(-10, 13),ScorePair(7, 12),ScorePair(20, 19),ScorePair(22, 15),ScorePair(-5, 9),ScorePair(2, 3),ScorePair(-7, -9),
ScorePair(-21, 4),ScorePair(-6, 4),ScorePair(39, 3),ScorePair(14, 14),ScorePair(48, 0),ScorePair(41, 12),ScorePair(31, 0),ScorePair(29, -10),
ScorePair(-46, -14),ScorePair(0, 2),ScorePair(-23, 4),ScorePair(-27, 0),ScorePair(-1, 1),ScorePair(41, -16),ScorePair(-13, 5),ScorePair(11, -34),
ScorePair(-42, -2),ScorePair(-62, -13),ScorePair(-57, -13),ScorePair(-80, 6),ScorePair(-130, 9),ScorePair(-51, -2),ScorePair(-69, -10),ScorePair(-102, 6),
ScorePair(-20, -3),ScorePair(-13, 2),ScorePair(2, 1),ScorePair(3, 9),ScorePair(9, -2),ScorePair(-5, -7),ScorePair(-25, 0),ScorePair(-6, -38),
ScorePair(-47, 2),ScorePair(-22, -1),ScorePair(-23, 9),ScorePair(-19, 2),ScorePair(-10, 0),ScorePair(-7, -1),ScorePair(11, -13),ScorePair(-47, -9),
ScorePair(-41, -2),ScorePair(-30, -3),ScorePair(-7, -4),ScorePair(-2, -4),ScorePair(-7, -2),ScorePair(-10, -10),ScorePair(14, -21),ScorePair(1, -17),
ScorePair(-33, 6),ScorePair(-37, 8),ScorePair(-30, 13),ScorePair(-16, 10),ScorePair(-4, 4),ScorePair(-11, 0),ScorePair(0, -12),ScorePair(-6, -18),
ScorePair(-32, 12),ScorePair(-15, 5),ScorePair(8, 7),ScorePair(21, 2),ScorePair(5, 3),ScorePair(17, 0),ScorePair(31, -11),ScorePair(32, -14),
ScorePair(-1, 7),ScorePair(-2, 11),ScorePair(2, 10),ScorePair(32, 0),ScorePair(54, -9),ScorePair(47, -7),ScorePair(63, -8),ScorePair(42, -12),
ScorePair(0, 11),ScorePair(-6, 22),ScorePair(31, 15),ScorePair(57, 6),ScorePair(23, 2),ScorePair(79, -4),ScorePair(71, -5),ScorePair(49, -11),
ScorePair(26, 16),ScorePair(31, 11),ScorePair(38, 13),ScorePair(49, 3),ScorePair(60, 2),ScorePair(47, -1),ScorePair(0, 0),ScorePair(19, 0),
ScorePair(6, -26),ScorePair(-9, -41),ScorePair(2, -32),ScorePair(17, -63),ScorePair(-2, -20),ScorePair(-25, -33),ScorePair(-61, -41),ScorePair(-7, -78),
ScorePair(-14, -15),ScorePair(14, -24),ScorePair(13, -22),ScorePair(12, -5),ScorePair(16, -11),ScorePair(33, -64),ScorePair(33, -77),ScorePair(13, -66),
ScorePair(-7, -50),ScorePair(11, -42),ScorePair(4, 9),ScorePair(6, -1),ScorePair(1, 11),ScorePair(14, 1),ScorePair(15, 7),ScorePair(0, -28),
ScorePair(-9, -17),ScorePair(0, -9),ScorePair(-6, 10),ScorePair(-1, 36),ScorePair(11, 24),ScorePair(4, 6),ScorePair(14, 7),ScorePair(0, -10),
ScorePair(-14, -33),ScorePair(-11, 10),ScorePair(-7, 7),ScorePair(-3, 37),ScorePair(-3, 50),ScorePair(6, 45),ScorePair(-6, 22),ScorePair(20, -4),
ScorePair(-13, -39),ScorePair(-16, -21),ScorePair(6, 19),ScorePair(29, 7),ScorePair(38, 48),ScorePair(74, -4),ScorePair(110, -33),ScorePair(69, -44),
ScorePair(-21, -8),ScorePair(-26, 6),ScorePair(-28, 19),ScorePair(-53, 60),ScorePair(-38, 42),ScorePair(80, 30),ScorePair(16, 24),ScorePair(118, -92),
ScorePair(-50, 6),ScorePair(-70, 37),ScorePair(-21, 23),ScorePair(-14, 36),ScorePair(-16, 32),ScorePair(-25, 44),ScorePair(-81, 20),ScorePair(-69, 43),
ScorePair(-40, -32),ScorePair(32, -32),ScorePair(1, -16),ScorePair(-73, -3),ScorePair(0, -30),ScorePair(-39, -7),ScorePair(45, -38),ScorePair(40, -62),
ScorePair(40, -32),ScorePair(-8, -5),ScorePair(-27, 11),ScorePair(-87, 30),ScorePair(-65, 28),ScorePair(-29, 18),ScorePair(30, -2),ScorePair(33, -21),
ScorePair(13, -27),ScorePair(21, -10),ScorePair(-57, 22),ScorePair(-77, 32),ScorePair(-78, 35),ScorePair(-50, 29),ScorePair(-5, 10),ScorePair(-25, -2),
ScorePair(26, -35),ScorePair(22, -8),ScorePair(12, 12),ScorePair(-68, 32),ScorePair(-37, 29),ScorePair(-63, 35),ScorePair(-28, 16),ScorePair(-66, 0),
ScorePair(14, -26),ScorePair(24, -5),ScorePair(48, 6),ScorePair(2, 19),ScorePair(-3, 20),ScorePair(-11, 26),ScorePair(11, 19),ScorePair(-25, 5),
ScorePair(71, -24),ScorePair(127, -3),ScorePair(46, 5),ScorePair(77, 4),ScorePair(25, 14),ScorePair(74, 19),ScorePair(86, 24),ScorePair(-9, 16),
ScorePair(55, -21),ScorePair(107, -7),ScorePair(90, -3),ScorePair(46, -2),ScorePair(42, 3),ScorePair(113, 11),ScorePair(2, 26),ScorePair(-15, 12),
ScorePair(51, -59),ScorePair(70, -31),ScorePair(58, -19),ScorePair(71, -25),ScorePair(78, -14),ScorePair(91, -3),ScorePair(46, -4),ScorePair(28, -32)});

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
