#ifndef QUIRKY_SRC_CORE_BOARD_GEOMETRY_H
#define QUIRKY_SRC_CORE_BOARD_GEOMETRY_H

#include <array>

#include "types.h"

namespace q_core {

inline static constexpr std::array<bitboard_t, BOARD_SIDE> RANK_BITBOARD = {
    0x00000000000000ff, 0x000000000000ff00, 0x0000000000ff0000, 0x00000000ff000000,
    0x000000ff00000000, 0x0000ff0000000000, 0x00ff000000000000, 0xff00000000000000};
inline static constexpr std::array<bitboard_t, BOARD_SIDE> FILE_BITBOARD = {
    0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080};
inline static constexpr std::array<bitboard_t, BOARD_SIDE * 2 - 1> LEFT_DIAGONAL_BITBOARD = {
    0x0000000000000001, 0x0000000000000102, 0x0000000000010204, 0x0000000001020408,
    0x0000000102040810, 0x0000010204081020, 0x0001020408102040, 0x0102040810204080,
    0x0204081020408000, 0x0408102040800000, 0x0810204080000000, 0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000};
inline static constexpr std::array<bitboard_t, BOARD_SIDE * 2 - 1> RIGHT_DIAGONAL_BITBOARD = {
    0x0000000000000080, 0x0000000000008040, 0x0000000000804020, 0x0000000080402010,
    0x0000008040201008, 0x0000804020100804, 0x0080402010080402, 0x8040201008040201,
    0x4020100804020100, 0x2010080402010000, 0x1008040201000000, 0x0804020100000000,
    0x0402010000000000, 0x0201000000000000, 0x0100000000000000};

constexpr bitboard_t FRAME_BITBOARD =
    RANK_BITBOARD[0] | RANK_BITBOARD[7] | FILE_BITBOARD[0] | FILE_BITBOARD[7];

inline constexpr subcoord_t MakeCoord(const subcoord_t rank, const subcoord_t file) {
    Q_ASSERT(IsSubcoordValid(rank) && IsSubcoordValid(file));
    return (rank << BOARD_SIDE_LOG) | file;
}

inline constexpr subcoord_t InvertSubcoord(const subcoord_t c) {
    Q_ASSERT(IsSubcoordValid(c));
    return BOARD_SIDE - 1 - c;
}

inline constexpr subcoord_t GetRank(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    return c >> BOARD_SIDE_LOG;
}

inline constexpr subcoord_t GetFile(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    return c & ((1 << BOARD_SIDE_LOG) - 1);
}

inline constexpr subcoord_t FlipCoord(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    return c ^ (BOARD_SIZE - BOARD_SIDE);
}

constexpr coord_t WHITE_KING_INITIAL_POSITION = MakeCoord(0, 4);
constexpr coord_t BLACK_KING_INITIAL_POSITION = MakeCoord(7, 4);

bitboard_t GetBitboardBetween(coord_t src, coord_t dst);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_BOARD_GEOMETRY_H
