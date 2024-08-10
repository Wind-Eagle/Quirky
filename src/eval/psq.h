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
    ScorePair(100, 100), ScorePair(308, 308), ScorePair(335, 335),
    ScorePair(521, 521), ScorePair(973, 973), ScorePair(0, 0)};

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
            ScorePair(-12, -12),   ScorePair(-3, -3),   ScorePair(-10, -10), ScorePair(-21, -21),
            ScorePair(1, 1),       ScorePair(18, 18),   ScorePair(24, 24),   ScorePair(-10, -10),
            ScorePair(-7, -7),     ScorePair(-8, -8),   ScorePair(-10, -10), ScorePair(-17, -17),
            ScorePair(-7, -7),     ScorePair(0, 0),     ScorePair(15, 15),   ScorePair(0, 0),
            ScorePair(-12, -12),   ScorePair(-12, -12), ScorePair(-11, -11), ScorePair(-9, -9),
            ScorePair(-18, -18),   ScorePair(0, 0),     ScorePair(-8, -8),   ScorePair(-18, -18),
            ScorePair(0, 0),       ScorePair(1, 1),     ScorePair(-14, -14), ScorePair(-10, -10),
            ScorePair(3, 3),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(-10, -10),
            ScorePair(31, 31),     ScorePair(29, 29),   ScorePair(4, 4),     ScorePair(-14, -14),
            ScorePair(-24, -24),   ScorePair(20, 20),   ScorePair(7, 7),     ScorePair(27, 27),
            ScorePair(302, 302),   ScorePair(264, 264), ScorePair(269, 269), ScorePair(166, 166),
            ScorePair(217, 217),   ScorePair(204, 204), ScorePair(171, 171), ScorePair(222, 222),
            ScorePair(0, 0),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-121, -121), ScorePair(-42, -42), ScorePair(-44, -44), ScorePair(-15, -15),
            ScorePair(-14, -14),   ScorePair(-9, -9),   ScorePair(-28, -28), ScorePair(-92, -92),
            ScorePair(-46, -46),   ScorePair(-36, -36), ScorePair(-10, -10), ScorePair(9, 9),
            ScorePair(6, 6),       ScorePair(0, 0),     ScorePair(0, 0),     ScorePair(-3, -3),
            ScorePair(-31, -31),   ScorePair(-5, -5),   ScorePair(11, 11),   ScorePair(18, 18),
            ScorePair(32, 32),     ScorePair(24, 24),   ScorePair(11, 11),   ScorePair(-20, -20),
            ScorePair(-19, -19),   ScorePair(-2, -2),   ScorePair(32, 32),   ScorePair(36, 36),
            ScorePair(37, 37),     ScorePair(35, 35),   ScorePair(23, 23),   ScorePair(17, 17),
            ScorePair(4, 4),       ScorePair(15, 15),   ScorePair(62, 62),   ScorePair(66, 66),
            ScorePair(64, 64),     ScorePair(75, 75),   ScorePair(57, 57),   ScorePair(35, 35),
            ScorePair(-17, -17),   ScorePair(-3, -3),   ScorePair(56, 56),   ScorePair(40, 40),
            ScorePair(72, 72),     ScorePair(75, 75),   ScorePair(36, 36),   ScorePair(8, 8),
            ScorePair(-40, -40),   ScorePair(-7, -7),   ScorePair(17, 17),   ScorePair(29, 29),
            ScorePair(65, 65),     ScorePair(43, 43),   ScorePair(-7, -7),   ScorePair(-41, -41),
            ScorePair(-85, -85),   ScorePair(-16, -16), ScorePair(-7, -7),   ScorePair(-30, -30),
            ScorePair(-2, -2),     ScorePair(-65, -65), ScorePair(-76, -76), ScorePair(-137, -137),
            ScorePair(-37, -37),   ScorePair(-4, -4),   ScorePair(-27, -27), ScorePair(-17, -17),
            ScorePair(-13, -13),   ScorePair(-10, -10), ScorePair(-35, -35), ScorePair(-79, -79),
            ScorePair(-10, -10),   ScorePair(-4, -4),   ScorePair(15, 15),   ScorePair(-4, -4),
            ScorePair(6, 6),       ScorePair(9, 9),     ScorePair(18, 18),   ScorePair(-36, -36),
            ScorePair(-10, -10),   ScorePair(29, 29),   ScorePair(15, 15),   ScorePair(19, 19),
            ScorePair(14, 14),     ScorePair(18, 18),   ScorePair(11, 11),   ScorePair(-2, -2),
            ScorePair(-5, -5),     ScorePair(0, 0),     ScorePair(31, 31),   ScorePair(26, 26),
            ScorePair(32, 32),     ScorePair(15, 15),   ScorePair(-2, -2),   ScorePair(-3, -3),
            ScorePair(-20, -20),   ScorePair(8, 8),     ScorePair(23, 23),   ScorePair(50, 50),
            ScorePair(50, 50),     ScorePair(32, 32),   ScorePair(2, 2),     ScorePair(-7, -7),
            ScorePair(-7, -7),     ScorePair(9, 9),     ScorePair(8, 8),     ScorePair(24, 24),
            ScorePair(36, 36),     ScorePair(16, 16),   ScorePair(45, 45),   ScorePair(27, 27),
            ScorePair(-46, -46),   ScorePair(4, 4),     ScorePair(0, 0),     ScorePair(-1, -1),
            ScorePair(-8, -8),     ScorePair(28, 28),   ScorePair(-39, -39), ScorePair(-12, -12),
            ScorePair(-55, -55),   ScorePair(-24, -24), ScorePair(-23, -23), ScorePair(-84, -84),
            ScorePair(10, 10),     ScorePair(-84, -84), ScorePair(31, 31),   ScorePair(-67, -67),
            ScorePair(-19, -19),   ScorePair(-14, -14), ScorePair(-23, -23), ScorePair(-25, -25),
            ScorePair(-20, -20),   ScorePair(-17, -17), ScorePair(-11, -11), ScorePair(-25, -25),
            ScorePair(-40, -40),   ScorePair(-45, -45), ScorePair(-32, -32), ScorePair(-35, -35),
            ScorePair(-26, -26),   ScorePair(-17, -17), ScorePair(-9, -9),   ScorePair(-36, -36),
            ScorePair(-31, -31),   ScorePair(-25, -25), ScorePair(-38, -38), ScorePair(-35, -35),
            ScorePair(-28, -28),   ScorePair(-26, -26), ScorePair(0, 0),     ScorePair(-20, -20),
            ScorePair(-10, -10),   ScorePair(-13, -13), ScorePair(-19, -19), ScorePair(-14, -14),
            ScorePair(-20, -20),   ScorePair(-11, -11), ScorePair(2, 2),     ScorePair(-33, -33),
            ScorePair(1, 1),       ScorePair(0, 0),     ScorePair(6, 6),     ScorePair(0, 0),
            ScorePair(5, 5),       ScorePair(23, 23),   ScorePair(8, 8),     ScorePair(14, 14),
            ScorePair(23, 23),     ScorePair(27, 27),   ScorePair(24, 24),   ScorePair(18, 18),
            ScorePair(25, 25),     ScorePair(52, 52),   ScorePair(56, 56),   ScorePair(36, 36),
            ScorePair(30, 30),     ScorePair(32, 32),   ScorePair(44, 44),   ScorePair(56, 56),
            ScorePair(54, 54),     ScorePair(51, 51),   ScorePair(60, 60),   ScorePair(55, 55),
            ScorePair(26, 26),     ScorePair(14, 14),   ScorePair(30, 30),   ScorePair(9, 9),
            ScorePair(43, 43),     ScorePair(32, 32),   ScorePair(51, 51),   ScorePair(59, 59),
            ScorePair(-36, -36),   ScorePair(-52, -52), ScorePair(-27, -27), ScorePair(-29, -29),
            ScorePair(-25, -25),   ScorePair(-77, -77), ScorePair(-92, -92), ScorePair(-91, -91),
            ScorePair(-44, -44),   ScorePair(-14, -14), ScorePair(-13, -13), ScorePair(-17, -17),
            ScorePair(-9, -9),     ScorePair(-22, -22), ScorePair(-29, -29), ScorePair(-55, -55),
            ScorePair(-45, -45),   ScorePair(-26, -26), ScorePair(-16, -16), ScorePair(-10, -10),
            ScorePair(0, 0),       ScorePair(-5, -5),   ScorePair(-2, -2),   ScorePair(-26, -26),
            ScorePair(-25, -25),   ScorePair(-12, -12), ScorePair(0, 0),     ScorePair(5, 5),
            ScorePair(14, 14),     ScorePair(11, 11),   ScorePair(4, 4),     ScorePair(4, 4),
            ScorePair(-20, -20),   ScorePair(-5, -5),   ScorePair(7, 7),     ScorePair(14, 14),
            ScorePair(30, 30),     ScorePair(21, 21),   ScorePair(28, 28),   ScorePair(0, 0),
            ScorePair(-29, -29),   ScorePair(-29, -29), ScorePair(24, 24),   ScorePair(5, 5),
            ScorePair(60, 60),     ScorePair(83, 83),   ScorePair(66, 66),   ScorePair(54, 54),
            ScorePair(-33, -33),   ScorePair(-25, -25), ScorePair(23, 23),   ScorePair(24, 24),
            ScorePair(64, 64),     ScorePair(44, 44),   ScorePair(6, 6),     ScorePair(30, 30),
            ScorePair(20, 20),     ScorePair(4, 4),     ScorePair(36, 36),   ScorePair(37, 37),
            ScorePair(63, 63),     ScorePair(67, 67),   ScorePair(40, 40),   ScorePair(42, 42),
            ScorePair(23, -77),    ScorePair(82, -50),  ScorePair(46, -29),  ScorePair(-16, -47),
            ScorePair(28, -55),    ScorePair(-9, -36),  ScorePair(64, -36),  ScorePair(65, -52),
            ScorePair(75, -49),    ScorePair(56, -45),  ScorePair(57, -38),  ScorePair(6, -22),
            ScorePair(19, -26),    ScorePair(8, -11),   ScorePair(39, -13),  ScorePair(59, -30),
            ScorePair(47, -52),    ScorePair(60, -33),  ScorePair(27, -22),  ScorePair(20, -23),
            ScorePair(-1, -15),    ScorePair(1, -9),    ScorePair(0, -11),   ScorePair(14, -21),
            ScorePair(-36, -17),   ScorePair(-23, -8),  ScorePair(0, -3),    ScorePair(-11, -2),
            ScorePair(-39, 9),     ScorePair(-50, 13),  ScorePair(-66, 8),   ScorePair(-91, 13),
            ScorePair(-120, 11),   ScorePair(-114, 30), ScorePair(-113, 31), ScorePair(-99, 31),
            ScorePair(-121, 39),   ScorePair(-178, 58), ScorePair(-176, 59), ScorePair(-147, 43),
            ScorePair(-146, 25),   ScorePair(-155, 46), ScorePair(-130, 41), ScorePair(-152, 48),
            ScorePair(-165, 65),   ScorePair(-176, 78), ScorePair(-218, 83), ScorePair(-244, 75),
            ScorePair(-53, 2),     ScorePair(-126, 35), ScorePair(-39, 21),  ScorePair(-136, 39),
            ScorePair(-82, 45),    ScorePair(-136, 63), ScorePair(-191, 68), ScorePair(-167, 43),
            ScorePair(9, -18),     ScorePair(-44, -2),  ScorePair(-81, 6),   ScorePair(-24, 0),
            ScorePair(-67, 21),    ScorePair(-75, 27),  ScorePair(-37, 20),  ScorePair(-105, 2)});

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
