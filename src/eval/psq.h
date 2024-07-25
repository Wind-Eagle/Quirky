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
    ScorePair(42, 157), ScorePair(151, 334), ScorePair(155, 369),
    ScorePair(169, 620), ScorePair(421, 1134), ScorePair(0, 0)};

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
    GetPSQ({ScorePair(0, 0),       ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),       ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-35, 15),    ScorePair(-13, 8),    ScorePair(-21, -2),  ScorePair(-24, -19),
            ScorePair(0, 0),       ScorePair(21, -12),   ScorePair(42, -21),  ScorePair(-8, -25),
            ScorePair(-28, -1),    ScorePair(-25, 0),    ScorePair(-37, -5),  ScorePair(-35, -13),
            ScorePair(-12, -17),   ScorePair(-4, -17),   ScorePair(25, -25),  ScorePair(7, -36),
            ScorePair(-40, 10),    ScorePair(-28, 10),   ScorePair(-15, -3),  ScorePair(-3, -22),
            ScorePair(-15, -34),   ScorePair(0, -24),    ScorePair(-6, -12),  ScorePair(-22, -18),
            ScorePair(-32, 38),    ScorePair(-19, 34),   ScorePair(-15, 10),  ScorePair(9, -10),
            ScorePair(25, -10),    ScorePair(7, -19),    ScorePair(5, -7),    ScorePair(-26, 0),
            ScorePair(5, 107),     ScorePair(18, 91),    ScorePair(30, 72),   ScorePair(87, 39),
            ScorePair(54, 37),     ScorePair(70, 22),    ScorePair(17, 61),   ScorePair(23, 62),
            ScorePair(106, 185),   ScorePair(65, 202),   ScorePair(129, 185), ScorePair(98, 144),
            ScorePair(162, 126),   ScorePair(57, 160),   ScorePair(-15, 188), ScorePair(42, 164),
            ScorePair(0, 0),       ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(0, 0),       ScorePair(0, 0),      ScorePair(0, 0),     ScorePair(0, 0),
            ScorePair(-171, -101), ScorePair(-62, -52),  ScorePair(-67, -53), ScorePair(-22, -21),
            ScorePair(-32, -34),   ScorePair(6, -36),    ScorePair(-20, -66), ScorePair(-137, -32),
            ScorePair(-61, -59),   ScorePair(-79, 6),    ScorePair(-36, 10),  ScorePair(0, -10),
            ScorePair(3, -18),     ScorePair(-17, -3),   ScorePair(10, -37),  ScorePair(17, -57),
            ScorePair(-44, -42),   ScorePair(-11, -28),  ScorePair(-5, 15),   ScorePair(-14, 17),
            ScorePair(8, 31),      ScorePair(10, 4),     ScorePair(2, -18),   ScorePair(-52, -10),
            ScorePair(-47, -3),    ScorePair(-27, 10),   ScorePair(0, 27),    ScorePair(25, 32),
            ScorePair(21, 21),     ScorePair(34, 12),    ScorePair(16, 2),    ScorePair(15, -14),
            ScorePair(-12, 0),     ScorePair(-10, 29),   ScorePair(49, 34),   ScorePair(71, 34),
            ScorePair(39, 34),     ScorePair(95, 19),    ScorePair(54, 26),   ScorePair(79, -49),
            ScorePair(-47, -5),    ScorePair(-23, 15),   ScorePair(60, 28),   ScorePair(21, 40),
            ScorePair(65, 39),     ScorePair(116, 0),    ScorePair(53, 6),    ScorePair(9, -20),
            ScorePair(-18, -33),   ScorePair(-14, 4),    ScorePair(14, 26),   ScorePair(15, 16),
            ScorePair(78, 21),     ScorePair(79, -10),   ScorePair(10, -8),   ScorePair(-31, -30),
            ScorePair(-38, -62),   ScorePair(31, -21),   ScorePair(-27, 30),  ScorePair(-79, 26),
            ScorePair(6, 2),       ScorePair(-65, 2),    ScorePair(-58, -40), ScorePair(-134, -58),
            ScorePair(-34, -56),   ScorePair(-12, -30),  ScorePair(-1, -50),  ScorePair(-26, -16),
            ScorePair(-24, -27),   ScorePair(4, -28),    ScorePair(-37, -40), ScorePair(-69, -49),
            ScorePair(50, -75),    ScorePair(-3, -15),   ScorePair(30, -21),  ScorePair(-4, 0),
            ScorePair(20, -12),    ScorePair(35, -20),   ScorePair(45, -28),  ScorePair(-26, -61),
            ScorePair(2, -38),     ScorePair(60, -21),   ScorePair(-3, 24),   ScorePair(12, 17),
            ScorePair(18, 8),      ScorePair(26, -1),    ScorePair(38, -32),  ScorePair(19, -39),
            ScorePair(21, -10),    ScorePair(-22, 20),   ScorePair(31, 35),   ScorePair(16, 24),
            ScorePair(37, 9),      ScorePair(20, 11),    ScorePair(-8, -8),   ScorePair(15, -54),
            ScorePair(-58, 25),    ScorePair(0, 9),      ScorePair(28, 25),   ScorePair(67, 12),
            ScorePair(51, 8),      ScorePair(45, -7),    ScorePair(23, 4),    ScorePair(0, -8),
            ScorePair(0, 0),       ScorePair(-21, 8),    ScorePair(-13, 28),  ScorePair(-7, 22),
            ScorePair(35, 6),      ScorePair(-1, 42),    ScorePair(62, -10),  ScorePair(59, -28),
            ScorePair(-45, -26),   ScorePair(1, 0),      ScorePair(-28, 19),  ScorePair(-19, 25),
            ScorePair(-42, 27),    ScorePair(21, 23),    ScorePair(-55, 5),   ScorePair(23, -23),
            ScorePair(-69, -14),   ScorePair(-66, 8),    ScorePair(-76, 15),  ScorePair(-154, 41),
            ScorePair(-21, 9),     ScorePair(-113, 10),  ScorePair(40, 2),    ScorePair(-131, 22),
            ScorePair(16, -15),    ScorePair(8, -21),    ScorePair(-10, -6),  ScorePair(-9, -8),
            ScorePair(2, -23),     ScorePair(19, -30),   ScorePair(56, -52),  ScorePair(41, -52),
            ScorePair(-2, -63),    ScorePair(-43, -26),  ScorePair(-42, -4),  ScorePair(-43, -14),
            ScorePair(-18, -18),   ScorePair(11, -41),   ScorePair(47, -81),  ScorePair(33, -74),
            ScorePair(-20, -19),   ScorePair(-29, 3),    ScorePair(-47, -2),  ScorePair(-39, -7),
            ScorePair(-22, -24),   ScorePair(-16, -15),  ScorePair(50, -53),  ScorePair(9, -31),
            ScorePair(-12, 4),     ScorePair(-38, 22),   ScorePair(-24, 8),   ScorePair(-20, 17),
            ScorePair(-4, -18),    ScorePair(-3, -6),    ScorePair(40, -34),  ScorePair(2, -49),
            ScorePair(-16, 24),    ScorePair(-22, 26),   ScorePair(-4, 27),   ScorePair(-20, 21),
            ScorePair(25, -12),    ScorePair(67, -16),   ScorePair(2, 14),    ScorePair(32, 10),
            ScorePair(-13, 31),    ScorePair(-31, 35),   ScorePair(-5, 30),   ScorePair(16, 10),
            ScorePair(43, -29),    ScorePair(93, -18),   ScorePair(60, 6),    ScorePair(49, 1),
            ScorePair(-2, 34),     ScorePair(-7, 41),    ScorePair(40, 16),   ScorePair(66, 16),
            ScorePair(87, 3),      ScorePair(65, 14),    ScorePair(72, 7),    ScorePair(104, 0),
            ScorePair(26, 0),      ScorePair(-27, 26),   ScorePair(-18, 34),  ScorePair(-66, 47),
            ScorePair(46, 4),      ScorePair(-29, 40),   ScorePair(60, 8),    ScorePair(119, 0),
            ScorePair(-25, -50),   ScorePair(13, -131),  ScorePair(-1, -88),  ScorePair(-9, -59),
            ScorePair(2, -89),     ScorePair(-42, -119), ScorePair(-2, -173), ScorePair(-46, -138),
            ScorePair(-13, -83),   ScorePair(-6, -59),   ScorePair(-4, -50),  ScorePair(-13, -39),
            ScorePair(6, -52),     ScorePair(8, -101),   ScorePair(40, -147), ScorePair(1, -136),
            ScorePair(-28, -52),   ScorePair(-14, -33),  ScorePair(-26, 7),   ScorePair(0, -24),
            ScorePair(0, 0),       ScorePair(-5, -21),   ScorePair(4, -22),   ScorePair(-2, -80),
            ScorePair(-17, -19),   ScorePair(5, -6),     ScorePair(-4, 34),   ScorePair(0, 54),
            ScorePair(3, 53),      ScorePair(-8, 55),    ScorePair(0, 11),    ScorePair(14, 1),
            ScorePair(5, -16),     ScorePair(29, -7),    ScorePair(24, 19),   ScorePair(4, 50),
            ScorePair(14, 68),     ScorePair(-9, 101),   ScorePair(-4, 71),   ScorePair(-14, 30),
            ScorePair(19, -59),    ScorePair(-14, 0),    ScorePair(24, 24),   ScorePair(-4, 61),
            ScorePair(40, 94),     ScorePair(83, 77),    ScorePair(76, 26),   ScorePair(56, 9),
            ScorePair(-18, 41),    ScorePair(-19, 16),   ScorePair(-7, 76),   ScorePair(-41, 97),
            ScorePair(26, 110),    ScorePair(39, 62),    ScorePair(-7, 52),   ScorePair(16, 67),
            ScorePair(68, -58),    ScorePair(-4, 11),    ScorePair(44, 25),   ScorePair(-14, 76),
            ScorePair(53, 30),     ScorePair(117, -6),   ScorePair(72, -29),  ScorePair(116, -94),
            ScorePair(-26, -17),   ScorePair(133, -65),  ScorePair(92, -52),  ScorePair(-83, -39),
            ScorePair(50, -76),    ScorePair(-29, -39),  ScorePair(128, -76), ScorePair(108, -99),
            ScorePair(69, -40),    ScorePair(26, -16),   ScorePair(39, -17),  ScorePair(-11, -12),
            ScorePair(-1, -10),    ScorePair(7, -13),    ScorePair(80, -32),  ScorePair(114, -59),
            ScorePair(9, -37),     ScorePair(43, -23),   ScorePair(-23, 1),   ScorePair(-60, 10),
            ScorePair(-65, 15),    ScorePair(-20, 0),    ScorePair(-12, -7),  ScorePair(10, -20),
            ScorePair(-62, -39),   ScorePair(-58, -5),   ScorePair(-45, 16),  ScorePair(-68, 24),
            ScorePair(-71, 25),    ScorePair(-82, 34),   ScorePair(-108, 26), ScorePair(-115, 17),
            ScorePair(-92, -1),    ScorePair(-71, 30),   ScorePair(-129, 22), ScorePair(-99, 28),
            ScorePair(-89, 49),    ScorePair(-213, 74),  ScorePair(-138, 71), ScorePair(-90, 35),
            ScorePair(-57, 11),    ScorePair(-70, 50),   ScorePair(-39, -24), ScorePair(-162, 39),
            ScorePair(-116, 47),   ScorePair(-71, 94),   ScorePair(-75, 98),  ScorePair(-90, 49),
            ScorePair(20, -5),     ScorePair(-10, -32),  ScorePair(17, 11),   ScorePair(-136, 2),
            ScorePair(-31, 62),    ScorePair(-48, 71),   ScorePair(-42, 112), ScorePair(-21, 46),
            ScorePair(88, -93),    ScorePair(-6, -26),   ScorePair(-30, -33), ScorePair(-11, 31),
            ScorePair(-31, 2),     ScorePair(-47, 52),   ScorePair(122, 45),  ScorePair(-11, -54)});

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
