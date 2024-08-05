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
    ScorePair(46, 153), ScorePair(189, 420), ScorePair(194, 470),
    ScorePair(209, 778), ScorePair(537, 1439), ScorePair(0, 0)};

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
    GetPSQ({ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),
ScorePair(-40, 27),ScorePair(-13, 13),ScorePair(-27, 8),ScorePair(-28, -13),ScorePair(3, 0),ScorePair(36, -2),ScorePair(55, -24),ScorePair(-7, -9),
ScorePair(-29, 14),ScorePair(-31, 3),ScorePair(-43, 6),ScorePair(-45, -7),ScorePair(-17, -9),ScorePair(1, -7),ScorePair(34, -17),ScorePair(16, -23),
ScorePair(-50, 22),ScorePair(-38, 18),ScorePair(-25, 0),ScorePair(-13, -19),ScorePair(-17, -33),ScorePair(-1, -11),ScorePair(-9, -13),ScorePair(-28, -10),
ScorePair(-37, 44),ScorePair(-17, 18),ScorePair(-23, 0),ScorePair(14, -35),ScorePair(39, -16),ScorePair(19, -24),ScorePair(17, -22),ScorePair(-17, 1),
ScorePair(-3, 52),ScorePair(7, 25),ScorePair(17, 10),ScorePair(47, -39),ScorePair(34, -39),ScorePair(87, -37),ScorePair(34, -12),ScorePair(25, 0),
ScorePair(143, 251),ScorePair(89, 266),ScorePair(171, 250),ScorePair(112, 194),ScorePair(217, 173),ScorePair(88, 223),ScorePair(-6, 257),ScorePair(61, 227),
ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),ScorePair(0, 0),
ScorePair(-198, -127),ScorePair(-73, -48),ScorePair(-69, -62),ScorePair(-32, -21),ScorePair(-36, -33),ScorePair(-2, -44),ScorePair(-33, -73),ScorePair(-178, -49),
ScorePair(-88, -72),ScorePair(-96, 10),ScorePair(-48, 20),ScorePair(0, -5),ScorePair(0, -7),ScorePair(-15, 3),ScorePair(5, -41),ScorePair(5, -82),
ScorePair(-66, -55),ScorePair(-3, -23),ScorePair(0, 24),ScorePair(-9, 35),ScorePair(0, 49),ScorePair(11, 16),ScorePair(1, -21),ScorePair(-66, -9),
ScorePair(-59, 1),ScorePair(-32, 17),ScorePair(10, 44),ScorePair(26, 42),ScorePair(28, 36),ScorePair(45, 16),ScorePair(19, 17),ScorePair(15, -8),
ScorePair(-17, -1),ScorePair(-10, 38),ScorePair(68, 49),ScorePair(95, 36),ScorePair(53, 47),ScorePair(116, 30),ScorePair(67, 28),ScorePair(99, -54),
ScorePair(-58, -12),ScorePair(-23, 21),ScorePair(74, 33),ScorePair(32, 52),ScorePair(80, 43),ScorePair(136, 1),ScorePair(54, 14),ScorePair(12, -10),
ScorePair(-30, -42),ScorePair(-19, -6),ScorePair(4, 28),ScorePair(15, 28),ScorePair(102, 42),ScorePair(95, 8),ScorePair(9, -19),ScorePair(-52, -31),
ScorePair(-37, -61),ScorePair(23, -47),ScorePair(-23, 41),ScorePair(-102, 28),ScorePair(7, -13),ScorePair(-91, -1),ScorePair(-83, -48),ScorePair(-162, -65),
ScorePair(-19, -46),ScorePair(-9, -40),ScorePair(-2, -70),ScorePair(-31, -18),ScorePair(-18, -28),ScorePair(-2, -44),ScorePair(-27, -43),ScorePair(-75, -60),
ScorePair(70, -72),ScorePair(-3, -23),ScorePair(42, -21),ScorePair(-5, -5),ScorePair(23, -17),ScorePair(43, -30),ScorePair(57, -41),ScorePair(-26, -78),
ScorePair(1, -40),ScorePair(81, -10),ScorePair(1, 29),ScorePair(14, 23),ScorePair(24, 20),ScorePair(36, 3),ScorePair(52, -41),ScorePair(21, -48),
ScorePair(15, -20),ScorePair(-21, 31),ScorePair(33, 39),ScorePair(20, 28),ScorePair(39, 2),ScorePair(34, 20),ScorePair(-10, -10),ScorePair(22, -56),
ScorePair(-68, 29),ScorePair(-3, 11),ScorePair(35, 31),ScorePair(89, 15),ScorePair(58, 6),ScorePair(62, -3),ScorePair(29, 5),ScorePair(-2, -20),
ScorePair(-1, -2),ScorePair(-22, 10),ScorePair(-15, 41),ScorePair(6, 36),ScorePair(45, 17),ScorePair(-14, 45),ScorePair(78, -11),ScorePair(73, -38),
ScorePair(-52, -31),ScorePair(8, -3),ScorePair(-45, 24),ScorePair(-17, 30),ScorePair(-68, 38),ScorePair(20, 17),ScorePair(-80, 20),ScorePair(13, -29),
ScorePair(-74, -20),ScorePair(-81, 7),ScorePair(-105, 23),ScorePair(-212, 44),ScorePair(-7, 22),ScorePair(-142, 21),ScorePair(53, 2),ScorePair(-150, 15),
ScorePair(23, -19),ScorePair(11, -15),ScorePair(-10, -3),ScorePair(-14, -8),ScorePair(0, -33),ScorePair(22, -31),ScorePair(65, -58),ScorePair(56, -52),
ScorePair(2, -63),ScorePair(-48, -24),ScorePair(-48, 1),ScorePair(-45, -9),ScorePair(-24, -24),ScorePair(17, -48),ScorePair(61, -87),ScorePair(41, -92),
ScorePair(-32, -11),ScorePair(-42, -6),ScorePair(-57, 0),ScorePair(-42, -6),ScorePair(-28, -19),ScorePair(-16, -14),ScorePair(66, -59),ScorePair(9, -24),
ScorePair(-10, 16),ScorePair(-45, 37),ScorePair(-41, 27),ScorePair(-37, 11),ScorePair(-14, -12),ScorePair(-8, -12),ScorePair(60, -34),ScorePair(4, -40),
ScorePair(-31, 40),ScorePair(-39, 34),ScorePair(-17, 44),ScorePair(-11, 27),ScorePair(40, -5),ScorePair(83, -7),ScorePair(0, 21),ScorePair(38, 10),
ScorePair(-22, 48),ScorePair(-38, 52),ScorePair(-6, 39),ScorePair(24, 21),ScorePair(53, -30),ScorePair(110, -16),ScorePair(68, 9),ScorePair(73, 8),
ScorePair(-5, 35),ScorePair(-10, 59),ScorePair(45, 29),ScorePair(85, 39),ScorePair(112, 7),ScorePair(77, 27),ScorePair(93, 12),ScorePair(127, 10),
ScorePair(42, -2),ScorePair(-40, 43),ScorePair(-22, 47),ScorePair(-85, 62),ScorePair(56, 8),ScorePair(-53, 57),ScorePair(83, 14),ScorePair(146, 0),
ScorePair(-28, -74),ScorePair(6, -174),ScorePair(-3, -109),ScorePair(-7, -80),ScorePair(16, -100),ScorePair(-58, -163),ScorePair(-21, -226),ScorePair(-49, -143),
ScorePair(-9, -98),ScorePair(-1, -78),ScorePair(0, -61),ScorePair(-9, -58),ScorePair(7, -62),ScorePair(14, -129),ScorePair(49, -201),ScorePair(-5, -185),
ScorePair(-44, -73),ScorePair(-24, -52),ScorePair(-27, 10),ScorePair(-3, -21),ScorePair(4, 12),ScorePair(0, -30),ScorePair(4, -21),ScorePair(-4, -115),
ScorePair(-11, -5),ScorePair(15, 6),ScorePair(-6, 42),ScorePair(-5, 67),ScorePair(9, 64),ScorePair(-19, 66),ScorePair(-5, 9),ScorePair(13, -3),
ScorePair(11, -14),ScorePair(28, -7),ScorePair(31, 23),ScorePair(9, 58),ScorePair(20, 84),ScorePair(-22, 119),ScorePair(-8, 96),ScorePair(-14, 44),
ScorePair(24, -59),ScorePair(-16, 2),ScorePair(37, 43),ScorePair(-1, 65),ScorePair(49, 117),ScorePair(95, 97),ScorePair(86, 37),ScorePair(53, -2),
ScorePair(-24, 58),ScorePair(-20, 15),ScorePair(7, 124),ScorePair(-48, 137),ScorePair(29, 140),ScorePair(54, 86),ScorePair(-16, 59),ScorePair(12, 82),
ScorePair(83, -75),ScorePair(-14, 12),ScorePair(51, 31),ScorePair(-25, 85),ScorePair(60, 34),ScorePair(147, -18),ScorePair(92, -35),ScorePair(148, -107),
ScorePair(-43, -26),ScorePair(160, -83),ScorePair(112, -65),ScorePair(-90, -38),ScorePair(64, -95),ScorePair(-37, -50),ScorePair(163, -95),ScorePair(141, -126),
ScorePair(79, -47),ScorePair(43, -18),ScorePair(53, -20),ScorePair(-19, -11),ScorePair(3, -16),ScorePair(18, -16),ScorePair(115, -39),ScorePair(148, -73),
ScorePair(2, -43),ScorePair(45, -33),ScorePair(-34, -3),ScorePair(-71, 17),ScorePair(-75, 27),ScorePair(-19, 9),ScorePair(-7, -9),ScorePair(19, -21),
ScorePair(-74, -34),ScorePair(-56, -7),ScorePair(-59, 24),ScorePair(-110, 30),ScorePair(-86, 30),ScorePair(-117, 42),ScorePair(-120, 26),ScorePair(-132, 14),
ScorePair(-90, -8),ScorePair(-87, 33),ScorePair(-162, 30),ScorePair(-114, 43),ScorePair(-111, 56),ScorePair(-270, 96),ScorePair(-171, 83),ScorePair(-104, 42),
ScorePair(-83, 7),ScorePair(-90, 63),ScorePair(-54, -34),ScorePair(-198, 51),ScorePair(-136, 46),ScorePair(-93, 118),ScorePair(-75, 115),ScorePair(-103, 64),
ScorePair(46, 4),ScorePair(-11, -39),ScorePair(25, 10),ScorePair(-175, -1),ScorePair(-42, 62),ScorePair(-70, 88),ScorePair(-60, 145),ScorePair(-47, 52),
ScorePair(99, -128),ScorePair(-7, -33),ScorePair(-55, -48),ScorePair(-27, 38),ScorePair(-34, 6),ScorePair(-49, 69),ScorePair(124, 43),ScorePair(-54, -78)});

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
