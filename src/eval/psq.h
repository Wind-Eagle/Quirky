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

inline constexpr std::array<ScorePair, q_core::NUMBER_OF_PIECES> PIECE_COST = {
    ScorePair(100, 100), ScorePair(260, 260), ScorePair(280, 280),
    ScorePair(450, 450), ScorePair(850, 850), ScorePair(0, 0)};

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
    GetPSQ({ScorePair(0, 0),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-5, -5),     ScorePair(0, 0),     ScorePair(-10, -10), ScorePair(-21, -21),
            ScorePair(1, 1),       ScorePair(8, 8),     ScorePair(11, 11),   ScorePair(-18, -18),
            ScorePair(-6, -6),     ScorePair(-6, -6),   ScorePair(-12, -12), ScorePair(-21, -21),
            ScorePair(-9, -9),     ScorePair(-9, -9),   ScorePair(3, 3),     ScorePair(-15, -15),
            ScorePair(-5, -5),     ScorePair(-2, -2),   ScorePair(-3, -3),   ScorePair(-6, -6),
            ScorePair(-20, -20),   ScorePair(-8, -8),   ScorePair(-8, -8),   ScorePair(-16, -16),
            ScorePair(14, 14),     ScorePair(11, 11),   ScorePair(-5, -5),   ScorePair(0, 0),
            ScorePair(9, 9),       ScorePair(-8, -8),   ScorePair(0, 0),     ScorePair(-5, -5),
            ScorePair(71, 71),     ScorePair(68, 68),   ScorePair(60, 60),   ScorePair(61, 61),
            ScorePair(43, 43),     ScorePair(37, 37),   ScorePair(41, 41),   ScorePair(49, 49),
            ScorePair(147, 147),   ScorePair(156, 156), ScorePair(164, 164), ScorePair(112, 112),
            ScorePair(134, 134),   ScorePair(117, 117), ScorePair(127, 127), ScorePair(114, 114),
            ScorePair(0, 0),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-121, -121), ScorePair(-51, -51), ScorePair(-46, -46), ScorePair(-20, -20),
            ScorePair(-16, -16),   ScorePair(-14, -14), ScorePair(-33, -33), ScorePair(-83, -83),
            ScorePair(-58, -58),   ScorePair(-26, -26), ScorePair(-6, -6),   ScorePair(1, 1),
            ScorePair(2, 2),       ScorePair(-1, -1),   ScorePair(-14, -14), ScorePair(-2, -2),
            ScorePair(-38, -38),   ScorePair(-14, -14), ScorePair(8, 8),     ScorePair(15, 15),
            ScorePair(17, 17),     ScorePair(17, 17),   ScorePair(0, 0),     ScorePair(-27, -27),
            ScorePair(-23, -23),   ScorePair(3, 3),     ScorePair(17, 17),   ScorePair(26, 26),
            ScorePair(29, 29),     ScorePair(24, 24),   ScorePair(16, 16),   ScorePair(10, 10),
            ScorePair(0, 0),       ScorePair(9, 9),     ScorePair(44, 44),   ScorePair(52, 52),
            ScorePair(42, 42),     ScorePair(51, 51),   ScorePair(45, 45),   ScorePair(14, 14),
            ScorePair(-13, -13),   ScorePair(0, 0),     ScorePair(37, 37),   ScorePair(32, 32),
            ScorePair(47, 47),     ScorePair(41, 41),   ScorePair(23, 23),   ScorePair(7, 7),
            ScorePair(-28, -28),   ScorePair(-6, -6),   ScorePair(14, 14),   ScorePair(17, 17),
            ScorePair(45, 45),     ScorePair(32, 32),   ScorePair(-7, -7),   ScorePair(-33, -33),
            ScorePair(-55, -55),   ScorePair(-15, -15), ScorePair(3, 3),     ScorePair(-2, -2),
            ScorePair(-5, -5),     ScorePair(-20, -20), ScorePair(-42, -42), ScorePair(-89, -89),
            ScorePair(-24, -24),   ScorePair(-11, -11), ScorePair(-28, -28), ScorePair(-16, -16),
            ScorePair(-19, -19),   ScorePair(-10, -10), ScorePair(-33, -33), ScorePair(-57, -57),
            ScorePair(-11, -11),   ScorePair(-8, -8),   ScorePair(10, 10),   ScorePair(0, 0),
            ScorePair(6, 6),       ScorePair(-2, -2),   ScorePair(10, 10),   ScorePair(-39, -39),
            ScorePair(-12, -12),   ScorePair(20, 20),   ScorePair(9, 9),     ScorePair(16, 16),
            ScorePair(17, 17),     ScorePair(13, 13),   ScorePair(13, 13),   ScorePair(-13, -13),
            ScorePair(-5, -5),     ScorePair(0, 0),     ScorePair(25, 25),   ScorePair(17, 17),
            ScorePair(18, 18),     ScorePair(15, 15),   ScorePair(-7, -7),   ScorePair(-9, -9),
            ScorePair(-19, -19),   ScorePair(3, 3),     ScorePair(16, 16),   ScorePair(33, 33),
            ScorePair(26, 26),     ScorePair(16, 16),   ScorePair(10, 10),   ScorePair(-7, -7),
            ScorePair(-4, -4),     ScorePair(0, 0),     ScorePair(6, 6),     ScorePair(15, 15),
            ScorePair(21, 21),     ScorePair(10, 10),   ScorePair(15, 15),   ScorePair(18, 18),
            ScorePair(-35, -35),   ScorePair(-1, -1),   ScorePair(3, 3),     ScorePair(11, 11),
            ScorePair(2, 2),       ScorePair(21, 21),   ScorePair(-20, -20), ScorePair(-9, -9),
            ScorePair(-23, -23),   ScorePair(-15, -15), ScorePair(-9, -9),   ScorePair(-20, -20),
            ScorePair(10, 10),     ScorePair(-34, -34), ScorePair(16, 16),   ScorePair(-18, -18),
            ScorePair(-6, -6),     ScorePair(-4, -4),   ScorePair(-13, -13), ScorePair(-14, -14),
            ScorePair(-6, -6),     ScorePair(-14, -14), ScorePair(-8, -8),   ScorePair(-9, -9),
            ScorePair(-25, -25),   ScorePair(-28, -28), ScorePair(-21, -21), ScorePair(-24, -24),
            ScorePair(-21, -21),   ScorePair(-17, -17), ScorePair(-11, -11), ScorePair(-19, -19),
            ScorePair(-15, -15),   ScorePair(-13, -13), ScorePair(-21, -21), ScorePair(-24, -24),
            ScorePair(-24, -24),   ScorePair(-22, -22), ScorePair(-2, -2),   ScorePair(-13, -13),
            ScorePair(-2, -2),     ScorePair(-7, -7),   ScorePair(-5, -5),   ScorePair(-6, -6),
            ScorePair(-13, -13),   ScorePair(-10, -10), ScorePair(-9, -9),   ScorePair(-18, -18),
            ScorePair(8, 8),       ScorePair(2, 2),     ScorePair(14, 14),   ScorePair(4, 4),
            ScorePair(7, 7),       ScorePair(26, 26),   ScorePair(5, 5),     ScorePair(9, 9),
            ScorePair(22, 22),     ScorePair(17, 17),   ScorePair(12, 12),   ScorePair(13, 13),
            ScorePair(4, 4),       ScorePair(22, 22),   ScorePair(18, 18),   ScorePair(20, 20),
            ScorePair(24, 24),     ScorePair(21, 21),   ScorePair(29, 29),   ScorePair(34, 34),
            ScorePair(32, 32),     ScorePair(30, 30),   ScorePair(23, 23),   ScorePair(32, 32),
            ScorePair(9, 9),       ScorePair(11, 11),   ScorePair(19, 19),   ScorePair(4, 4),
            ScorePair(22, 22),     ScorePair(14, 14),   ScorePair(23, 23),   ScorePair(34, 34),
            ScorePair(-36, -36),   ScorePair(-44, -44), ScorePair(-24, -24), ScorePair(-26, -26),
            ScorePair(-32, -32),   ScorePair(-78, -78), ScorePair(-89, -89), ScorePair(-74, -74),
            ScorePair(-36, -36),   ScorePair(-21, -21), ScorePair(-7, -7),   ScorePair(-19, -19),
            ScorePair(-11, -11),   ScorePair(-29, -29), ScorePair(-32, -32), ScorePair(-58, -58),
            ScorePair(-44, -44),   ScorePair(-22, -22), ScorePair(-14, -14), ScorePair(-3, -3),
            ScorePair(0, 0),       ScorePair(-4, -4),   ScorePair(-2, -2),   ScorePair(-22, -22),
            ScorePair(-24, -24),   ScorePair(-5, -5),   ScorePair(3, 3),     ScorePair(13, 13),
            ScorePair(17, 17),     ScorePair(15, 15),   ScorePair(7, 7),     ScorePair(8, 8),
            ScorePair(-20, -20),   ScorePair(3, 3),     ScorePair(16, 16),   ScorePair(22, 22),
            ScorePair(36, 36),     ScorePair(24, 24),   ScorePair(31, 31),   ScorePair(5, 5),
            ScorePair(-19, -19),   ScorePair(-13, -13), ScorePair(23, 23),   ScorePair(14, 14),
            ScorePair(61, 61),     ScorePair(80, 80),   ScorePair(57, 57),   ScorePair(55, 55),
            ScorePair(-11, -11),   ScorePair(-15, -15), ScorePair(26, 26),   ScorePair(22, 22),
            ScorePair(57, 57),     ScorePair(44, 44),   ScorePair(15, 15),   ScorePair(39, 39),
            ScorePair(0, 0),       ScorePair(-1, -1),   ScorePair(24, 24),   ScorePair(30, 30),
            ScorePair(53, 53),     ScorePair(45, 45),   ScorePair(19, 19),   ScorePair(19, 19),
            ScorePair(-29, -20),   ScorePair(116, -64), ScorePair(75, -41),  ScorePair(-78, -23),
            ScorePair(38, -67),    ScorePair(-38, -27), ScorePair(120, -69), ScorePair(109, -90),
            ScorePair(61, -43),    ScorePair(43, -17),  ScorePair(45, -23),  ScorePair(-10, -14),
            ScorePair(-2, -18),    ScorePair(14, -15),  ScorePair(69, -27),  ScorePair(118, -62),
            ScorePair(21, -38),    ScorePair(40, -21),  ScorePair(-5, -9),   ScorePair(-42, -2),
            ScorePair(-46, 5),     ScorePair(-15, -4),  ScorePair(-9, -10),  ScorePair(3, -16),
            ScorePair(-68, -16),   ScorePair(-48, 2),   ScorePair(-29, 3),   ScorePair(-44, 16),
            ScorePair(-60, 21),    ScorePair(-78, 28),  ScorePair(-90, 21),  ScorePair(-99, 10),
            ScorePair(-101, 12),   ScorePair(-90, 29),  ScorePair(-119, 26), ScorePair(-90, 28),
            ScorePair(-77, 43),    ScorePair(-183, 64), ScorePair(-132, 59), ScorePair(-94, 35),
            ScorePair(-74, 26),    ScorePair(-68, 55),  ScorePair(-54, -10), ScorePair(-146, 39),
            ScorePair(-117, 43),   ScorePair(-66, 83),  ScorePair(-81, 87),  ScorePair(-114, 45),
            ScorePair(13, 12),     ScorePair(-27, -11), ScorePair(21, 9),    ScorePair(-125, 4),
            ScorePair(-15, 51),    ScorePair(-28, 65),  ScorePair(-48, 100), ScorePair(-57, 43),
            ScorePair(96, -69),    ScorePair(12, -8),   ScorePair(-18, -24), ScorePair(8, 21),
            ScorePair(-11, 8),     ScorePair(-24, 41),  ScorePair(89, 36),   ScorePair(-35, -40)});

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
