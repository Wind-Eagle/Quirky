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
    ScorePair(91, 108),  ScorePair(403, 233),  ScorePair(450, 260),
    ScorePair(564, 478), ScorePair(914, 1024), ScorePair(0, 0)};

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
            ScorePair(-41, 9),   ScorePair(2, -1),     ScorePair(-15, 0),    ScorePair(-20, 0),
            ScorePair(-1, -8),   ScorePair(27, -12),   ScorePair(56, -18),   ScorePair(-6, -19),
            ScorePair(-38, -1),  ScorePair(-11, -4),   ScorePair(-2, -22),   ScorePair(-8, -10),
            ScorePair(10, -16),  ScorePair(-12, -14),  ScorePair(36, -17),   ScorePair(-8, -17),
            ScorePair(-43, 6),   ScorePair(-5, -3),    ScorePair(-10, -19),  ScorePair(12, -23),
            ScorePair(10, -23),  ScorePair(0, -23),    ScorePair(4, -13),    ScorePair(-44, -13),
            ScorePair(-23, 26),  ScorePair(-1, 15),    ScorePair(-5, 2),     ScorePair(7, -11),
            ScorePair(20, -20),  ScorePair(1, -10),    ScorePair(5, 3),      ScorePair(-25, 6),
            ScorePair(-38, 100), ScorePair(-8, 95),    ScorePair(7, 77),     ScorePair(4, 57),
            ScorePair(28, 43),   ScorePair(57, 38),    ScorePair(0, 64),     ScorePair(-23, 79),
            ScorePair(31, 171),  ScorePair(27, 165),   ScorePair(23, 152),   ScorePair(51, 115),
            ScorePair(60, 108),  ScorePair(50, 125),   ScorePair(1, 161),    ScorePair(-12, 151),
            ScorePair(0, 0),     ScorePair(0, 0),      ScorePair(0, 0),      ScorePair(0, 0),
            ScorePair(0, 0),     ScorePair(0, 0),      ScorePair(0, 0),      ScorePair(0, 0),
            ScorePair(-71, -37), ScorePair(-21, -33),  ScorePair(-56, 0),    ScorePair(-34, 6),
            ScorePair(-34, -2),  ScorePair(-16, 3),    ScorePair(-25, -28),  ScorePair(-88, -24),
            ScorePair(-24, -19), ScorePair(-31, -4),   ScorePair(-3, -1),    ScorePair(1, 14),
            ScorePair(2, 9),     ScorePair(8, 6),      ScorePair(-3, -11),   ScorePair(-7, -19),
            ScorePair(-19, -10), ScorePair(-4, 11),    ScorePair(20, 6),     ScorePair(11, 32),
            ScorePair(23, 30),   ScorePair(14, 11),    ScorePair(23, -2),    ScorePair(-23, -12),
            ScorePair(-13, 2),   ScorePair(-1, 11),    ScorePair(15, 32),    ScorePair(14, 36),
            ScorePair(26, 38),   ScorePair(24, 24),    ScorePair(3, 5),      ScorePair(-26, 4),
            ScorePair(4, 0),     ScorePair(18, 26),    ScorePair(13, 30),    ScorePair(60, 43),
            ScorePair(26, 37),   ScorePair(49, 26),    ScorePair(4, 22),     ScorePair(21, -7),
            ScorePair(-5, -13),  ScorePair(15, 0),     ScorePair(27, 28),    ScorePair(70, 22),
            ScorePair(85, 0),    ScorePair(102, 4),    ScorePair(53, -7),    ScorePair(-11, -6),
            ScorePair(-77, -16), ScorePair(-35, -2),   ScorePair(72, -12),   ScorePair(19, 8),
            ScorePair(72, -14),  ScorePair(65, -15),   ScorePair(-8, -12),   ScorePair(-10, -42),
            ScorePair(-236, -7), ScorePair(-80, -38),  ScorePair(-56, -12),  ScorePair(-41, 0),
            ScorePair(11, -17),  ScorePair(-119, -11), ScorePair(-138, -18), ScorePair(-135, -72),
            ScorePair(-6, -26),  ScorePair(-27, -11),  ScorePair(-8, -26),   ScorePair(-25, 0),
            ScorePair(-33, 0),   ScorePair(-27, -11),  ScorePair(-22, -19),  ScorePair(-20, -15),
            ScorePair(-31, -4),  ScorePair(24, -21),   ScorePair(1, -6),     ScorePair(-2, 3),
            ScorePair(0, 7),     ScorePair(21, -8),    ScorePair(33, -10),   ScorePair(5, -27),
            ScorePair(12, -19),  ScorePair(12, 0),     ScorePair(14, 7),     ScorePair(5, 12),
            ScorePair(12, 20),   ScorePair(11, 6),     ScorePair(7, -9),     ScorePair(12, -18),
            ScorePair(-19, -4),  ScorePair(-5, 5),     ScorePair(2, 16),     ScorePair(25, 9),
            ScorePair(30, 13),   ScorePair(8, 17),     ScorePair(1, -7),     ScorePair(-36, -5),
            ScorePair(-40, 12),  ScorePair(-10, 14),   ScorePair(9, 10),     ScorePair(22, 19),
            ScorePair(24, 19),   ScorePair(-6, 9),     ScorePair(2, 3),      ScorePair(-13, -8),
            ScorePair(-20, 7),   ScorePair(-5, 7),     ScorePair(41, 1),     ScorePair(12, 12),
            ScorePair(52, 0),    ScorePair(42, 11),    ScorePair(34, 0),     ScorePair(29, -9),
            ScorePair(-39, -13), ScorePair(5, 1),      ScorePair(-20, 2),    ScorePair(-20, 0),
            ScorePair(0, -1),    ScorePair(44, -13),   ScorePair(-5, 2),     ScorePair(11, -35),
            ScorePair(-42, -3),  ScorePair(-60, -12),  ScorePair(-53, -12),  ScorePair(-79, 7),
            ScorePair(-131, 10), ScorePair(-52, -4),   ScorePair(-70, -10),  ScorePair(-98, 6),
            ScorePair(-20, -3),  ScorePair(-17, 4),    ScorePair(0, 0),      ScorePair(1, 11),
            ScorePair(6, -6),    ScorePair(-6, -8),    ScorePair(-27, -1),   ScorePair(-6, -37),
            ScorePair(-47, 2),   ScorePair(-22, 0),    ScorePair(-28, 6),    ScorePair(-20, 4),
            ScorePair(-14, 0),   ScorePair(-8, -3),    ScorePair(13, -14),   ScorePair(-49, -7),
            ScorePair(-41, -5),  ScorePair(-27, 0),    ScorePair(-7, -6),    ScorePair(-3, -2),
            ScorePair(-7, -3),   ScorePair(-9, -10),   ScorePair(12, -19),   ScorePair(-2, -20),
            ScorePair(-37, 3),   ScorePair(-37, 10),   ScorePair(-32, 13),   ScorePair(-16, 10),
            ScorePair(-8, 2),    ScorePair(-14, -3),   ScorePair(-4, -9),    ScorePair(-5, -18),
            ScorePair(-33, 11),  ScorePair(-15, 3),    ScorePair(6, 6),      ScorePair(16, 0),
            ScorePair(3, 5),     ScorePair(16, -1),    ScorePair(27, -6),    ScorePair(32, -15),
            ScorePair(-2, 9),    ScorePair(0, 10),     ScorePair(4, 9),      ScorePair(32, 0),
            ScorePair(55, -12),  ScorePair(50, -4),    ScorePair(63, -7),    ScorePair(39, -14),
            ScorePair(2, 13),    ScorePair(0, 24),     ScorePair(34, 11),    ScorePair(59, 6),
            ScorePair(26, 2),    ScorePair(80, -4),    ScorePair(71, -8),    ScorePair(44, -12),
            ScorePair(27, 16),   ScorePair(27, 9),     ScorePair(36, 10),    ScorePair(52, 6),
            ScorePair(66, 4),    ScorePair(51, -2),    ScorePair(4, 3),      ScorePair(15, 2),
            ScorePair(4, -26),   ScorePair(-8, -39),   ScorePair(0, -33),    ScorePair(19, -62),
            ScorePair(-6, -20),  ScorePair(-26, -33),  ScorePair(-62, -40),  ScorePair(-14, -77),
            ScorePair(-16, -16), ScorePair(13, -23),   ScorePair(11, -25),   ScorePair(10, -6),
            ScorePair(16, -14),  ScorePair(33, -65),   ScorePair(34, -77),   ScorePair(17, -70),
            ScorePair(-8, -48),  ScorePair(11, -42),   ScorePair(4, 9),      ScorePair(6, 0),
            ScorePair(1, 9),     ScorePair(12, 0),     ScorePair(14, 5),     ScorePair(0, -30),
            ScorePair(-12, -17), ScorePair(1, -9),     ScorePair(-4, 9),     ScorePair(0, 31),
            ScorePair(11, 20),   ScorePair(3, 2),      ScorePair(16, 6),     ScorePair(-3, -11),
            ScorePair(-18, -34), ScorePair(-14, 8),    ScorePair(-7, 4),     ScorePair(-3, 35),
            ScorePair(0, 49),    ScorePair(3, 43),     ScorePair(-5, 21),    ScorePair(21, -3),
            ScorePair(-15, -40), ScorePair(-13, -17),  ScorePair(9, 18),     ScorePair(33, 3),
            ScorePair(37, 47),   ScorePair(76, -12),   ScorePair(110, -34),  ScorePair(68, -44),
            ScorePair(-18, -9),  ScorePair(-27, 3),    ScorePair(-24, 15),   ScorePair(-53, 60),
            ScorePair(-40, 39),  ScorePair(81, 29),    ScorePair(22, 19),    ScorePair(118, -100),
            ScorePair(-46, 4),   ScorePair(-72, 37),   ScorePair(-15, 21),   ScorePair(-13, 36),
            ScorePair(-12, 31),  ScorePair(-28, 37),   ScorePair(-87, 18),   ScorePair(-73, 42),
            ScorePair(-41, -34), ScorePair(32, -31),   ScorePair(3, -15),    ScorePair(-74, -6),
            ScorePair(1, -31),   ScorePair(-38, -8),   ScorePair(46, -40),   ScorePair(41, -64),
            ScorePair(43, -32),  ScorePair(-7, -7),    ScorePair(-26, 10),   ScorePair(-85, 30),
            ScorePair(-66, 26),  ScorePair(-29, 20),   ScorePair(33, -2),    ScorePair(34, -22),
            ScorePair(12, -31),  ScorePair(26, -7),    ScorePair(-61, 19),   ScorePair(-77, 32),
            ScorePair(-75, 37),  ScorePair(-51, 29),   ScorePair(-1, 9),     ScorePair(-24, -2),
            ScorePair(26, -38),  ScorePair(21, -5),    ScorePair(13, 11),    ScorePair(-72, 30),
            ScorePair(-35, 30),  ScorePair(-66, 35),   ScorePair(-28, 17),   ScorePair(-65, 2),
            ScorePair(14, -27),  ScorePair(28, -3),    ScorePair(49, 5),     ScorePair(0, 17),
            ScorePair(-7, 18),   ScorePair(-10, 27),   ScorePair(11, 19),    ScorePair(-23, 7),
            ScorePair(66, -20),  ScorePair(134, 0),    ScorePair(49, 7),     ScorePair(76, 3),
            ScorePair(21, 13),   ScorePair(74, 21),    ScorePair(87, 26),    ScorePair(-16, 17),
            ScorePair(55, -24),  ScorePair(109, -5),   ScorePair(84, -7),    ScorePair(52, -3),
            ScorePair(41, 3),    ScorePair(112, 11),   ScorePair(4, 28),     ScorePair(-18, 13),
            ScorePair(53, -58),  ScorePair(75, -32),   ScorePair(56, -20),   ScorePair(66, -28),
            ScorePair(72, -15),  ScorePair(95, -5),    ScorePair(42, -5),    ScorePair(26, -34)});

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
