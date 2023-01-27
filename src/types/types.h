#ifndef QUIRKY_SRC_TYPES_TYPES_H
#define QUIRKY_SRC_TYPES_TYPES_H

#include <cstdint>

#define ENUM_TO_INT_OP(type, base, op)                                          \
    inline constexpr type operator op(const type a, const type b) {             \
        return static_cast<type>(static_cast<base>(a) op static_cast<base>(b)); \
    }

#define ENUM_ASSIGNMENT_OP(type, op)                               \
    inline constexpr type &operator op##=(type &a, const type b) { \
        a = a op b;                                                \
        return a;                                                  \
    }

#define ENUM_TO_INT(type, base)   \
    ENUM_TO_INT_OP(type, base, &) \
    ENUM_TO_INT_OP(type, base, |) \
    ENUM_TO_INT_OP(type, base, ^) \
    ENUM_ASSIGNMENT_OP(type, &)   \
    ENUM_ASSIGNMENT_OP(type, |)   \
    ENUM_ASSIGNMENT_OP(type, ^)   \
    inline constexpr type operator~(const type a) { return type::All ^ a; }

namespace core {

using coord_t = int8_t;
using subcoord_t = int8_t;
using cell_t = int8_t;
using bitboard_t = uint64_t;
using hash_t = uint64_t;

enum class Castling : uint8_t {
    None = 0,
    WhiteKingSide = 1,
    WhiteQueenSide = 2,
    BlackKingSide = 4,
    BlackQueenSide = 8,
    All = 15
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

constexpr cell_t EMPTY_CELL = 0;
constexpr cell_t INVALID_CELL = -1;

constexpr cell_t INVALID_COORD = -1;

constexpr int8_t NUMBER_OF_PIECES = 6;
constexpr int8_t NUMBER_OF_CELLS = 13;

}  // namespace core

#endif  // QUIRKY_SRC_TYPES_TYPES_H
