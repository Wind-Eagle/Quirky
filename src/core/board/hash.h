#ifndef QUIRKY_SRC_CORE_BOARD_HASH_H
#define QUIRKY_SRC_CORE_BOARD_HASH_H

#include <array>

#include "../../util/random.h"

namespace {

constexpr std::array<core::hash_t, 64 * 13> ZOBRIST_HASH_CELLS =
    util::MakeRandomArray64<64 * 13>("ZOBRIST_HASH_CELLS");
constexpr std::array<core::hash_t, 64> ZOBRIST_HASH_EN_PASSANT_COORD =
    util::MakeRandomArray64<64>("ZOBRIST_HASH_EN_PASSANT_COORD");
constexpr std::array<core::hash_t, 16> ZOBRIST_HASH_CASTLING =
    util::MakeRandomArray64<16>("ZOBRIST_HASH_CASTLING");
constexpr std::array<core::hash_t, 2> ZOBRIST_HASH_MOVE_SIDE =
    util::MakeRandomArray64<2>("ZOBRIST_HASH_MOVE_SIDE");

}  // namespace

namespace core {

inline constexpr hash_t MakeZobristHashFromCell(const coord_t x, const cell_t c) {
    return ZOBRIST_HASH_CELLS[static_cast<int>(c) * 64 + x];
}

inline constexpr hash_t MakeZobristHashFromEnPassantCoord(const coord_t c) {
    return ZOBRIST_HASH_EN_PASSANT_COORD[c];
}

inline constexpr hash_t MakeZobristHashFromCastling(const Castling c) {
    return ZOBRIST_HASH_CASTLING[static_cast<int>(c)];
}

inline constexpr hash_t MakeZobristHashFromMoveSide(const Color c) {
    return ZOBRIST_HASH_MOVE_SIDE[static_cast<int>(c)];
}

}  // namespace core

#endif  // QUIRKY_SRC_CORE_BOARD_HASH_H
