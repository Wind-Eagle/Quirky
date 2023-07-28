#ifndef QUIRKY_SRC_CORE_BOARD_TYPES_H
#define QUIRKY_SRC_CORE_BOARD_TYPES_H

#include <cstdint>

#include "../../util/macro.h"

namespace q_core {

constexpr uint8_t BOARD_SIDE = 8;
constexpr uint8_t BOARD_SIZE = BOARD_SIDE * BOARD_SIDE;
constexpr uint8_t BOARD_SIDE_LOG = 3;
constexpr uint8_t BOARD_SIZE_LOG = BOARD_SIDE_LOG * 2;

using coord_t = int8_t;
using subcoord_t = int8_t;
using cell_t = int8_t;
using bitboard_t = uint64_t;
using hash_t = uint64_t;

enum class Castling : uint8_t {
    None = 0,
    WhiteKingside = 1,
    WhiteQueenside = 2,
    BlackKingside = 4,
    BlackQueenside = 8,
    All = 15,
    WhiteAll = WhiteKingside | WhiteQueenside,
    BlackAll = BlackKingside | BlackQueenside
};

ENUM_TO_INT(Castling, uint8_t)

enum class Color : int8_t { White = 0, Black = 1 };

enum class Piece : int8_t {
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6,
};

constexpr int8_t NUMBER_OF_PIECES = 6;
constexpr int8_t NUMBER_OF_CELLS = 13;

constexpr cell_t EMPTY_CELL = 0;
constexpr cell_t UNDEFINED_CELL = -1;

constexpr coord_t UNDEFINED_COORD = -1;

inline constexpr bool IsSubcoordValid(const subcoord_t c) { return c >= 0 && c < BOARD_SIDE; }

inline constexpr bool IsCoordValid(const subcoord_t c) {
    return (c == UNDEFINED_COORD) || (c >= 0 && c < BOARD_SIZE);
}

inline constexpr bool IsCoordValidAndDefined(const subcoord_t c) {
    return c >= 0 && c < BOARD_SIZE;
}

inline constexpr bool IsColorValid(const Color c) {
    return static_cast<int8_t>(c) == 0 || static_cast<int8_t>(c) == 1;
}

inline constexpr bool IsPieceValid(const Piece p) {
    return static_cast<int8_t>(p) >= static_cast<int8_t>(Piece::Pawn) ||
           static_cast<int8_t>(p) <= static_cast<int8_t>(Piece::King);
}

inline constexpr bool IsCellValid(const cell_t c) {
    return static_cast<int8_t>(c) == UNDEFINED_CELL ||
           (static_cast<int8_t>(c) >= 0 && static_cast<int8_t>(c) < NUMBER_OF_CELLS);
}

inline constexpr bool IsCastlingValid(const Castling c) {
    return c >= Castling::None && c <= Castling::All;
}

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_BOARD_TYPES_H
