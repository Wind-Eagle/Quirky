#ifndef QUIRKY_SRC_CORE_BOARD_HASH_H
#define QUIRKY_SRC_CORE_BOARD_HASH_H

#include <array>

#include "../../util/hash.h"
#include "../../util/macro.h"
#include "../../util/random.h"
#include "types.h"

namespace {

template <size_t N>
inline constexpr std::array<uint64_t, N> MakeRandomArray64(const std::string_view& name) {
    std::array<uint64_t, N> res;
    uint64_t x = q_util::GetStringHash(name);
    for (uint32_t i = 0; i < N; i++) {
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        x *= 0x2545F4914F6CDD1DULL;
        res[i] = x;
    }
    return res;
}

constexpr std::array<q_core::hash_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_CELLS>
MakeZobristHashCells(
    const std::array<q_core::hash_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_CELLS> arr) {
    std::array<q_core::hash_t, q_core::BOARD_SIZE* q_core::NUMBER_OF_CELLS> ans = arr;
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        ans[(q_core::EMPTY_CELL << q_core::BOARD_SIZE_LOG) + i] = 0;
    }
    return ans;
}

}  // namespace

namespace q_core {

constexpr std::array<hash_t, BOARD_SIZE* NUMBER_OF_CELLS> ZOBRIST_HASH_CELLS = MakeZobristHashCells(
    MakeRandomArray64<BOARD_SIZE * NUMBER_OF_CELLS>(Q_UNIQUE_STRING("ZOBRIST")));
constexpr std::array<hash_t, BOARD_SIZE> ZOBRIST_HASH_EN_PASSANT_COORD =
    MakeRandomArray64<BOARD_SIZE>(Q_UNIQUE_STRING("ZOBRIST"));
constexpr std::array<hash_t, static_cast<uint8_t>(Castling::All) + 1> ZOBRIST_HASH_CASTLING =
    MakeRandomArray64<static_cast<uint8_t>(Castling::All) + 1>(Q_UNIQUE_STRING("ZOBRIST"));
constexpr std::array<hash_t, 2> ZOBRIST_HASH_MOVE_SIDE =
    MakeRandomArray64<2>(Q_UNIQUE_STRING("ZOBRIST"));

inline constexpr hash_t MakeZobristHashFromCell(const coord_t coord, const cell_t cell) {
    Q_ASSERT(IsCoordValid(coord) && IsCellValid(cell));
    return ZOBRIST_HASH_CELLS[(static_cast<int>(cell) << BOARD_SIZE_LOG) + coord];
}

inline constexpr hash_t MakeZobristHashFromEnPassantCoord(const coord_t c) {
    Q_ASSERT(IsCoordValidAndDefined(c));
    return ZOBRIST_HASH_EN_PASSANT_COORD[c];
}

inline constexpr hash_t MakeZobristHashFromCastling(const Castling c) {
    Q_ASSERT(IsCastlingValid(c));
    return ZOBRIST_HASH_CASTLING[static_cast<uint8_t>(c)];
}

inline constexpr hash_t MakeZobristHashFromMoveSide(const Color c) {
    Q_ASSERT(IsColorValid(c));
    return ZOBRIST_HASH_MOVE_SIDE[static_cast<int8_t>(c)];
}

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_BOARD_HASH_H
