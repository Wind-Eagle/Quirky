#ifndef QUIRKY_SRC_CORE_BOARD_UTIL_H
#define QUIRKY_SRC_CORE_BOARD_UTIL_H

#include "board/geometry.h"
#include "board/types.h"

namespace {

constexpr int8_t COLOR_OFFSET = static_cast<int8_t>(q_core::Piece::King);

}  // namespace

namespace q_core {

inline constexpr cell_t MakeCell(const Color c, const Piece p) {
    Q_ASSERT(IsColorValid(c) && IsPieceValid(p));
    return (c == Color::White ? 0 : COLOR_OFFSET) + static_cast<cell_t>(p);
}

inline constexpr bool IsCellWithPiece(const cell_t c) {
    Q_ASSERT(IsCellValid(c));
    return c != UNDEFINED_CELL && c != EMPTY_CELL;
}

inline constexpr Color GetCellColor(const cell_t c) {
    Q_ASSERT(IsCellValid(c) && IsCellWithPiece(c));
    return c <= COLOR_OFFSET ? Color::White : Color::Black;
}

inline constexpr bool IsAnyCastlingAllowed(const Castling c) {
    Q_ASSERT(IsCastlingValid(c));
    return c == Castling::None;
}

inline constexpr bool IsCastlingAllowed(const Castling c, const Castling p) {
    Q_ASSERT(IsCastlingValid(c) && IsCastlingValid(p));
    return (c & p) != Castling::None;
}

inline constexpr bool IsCoordStringValid(const std::string_view& str) {
    if (str.size() != 2) {
        return false;
    }
    return str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8';
}

inline constexpr coord_t CastStringToCoord(const std::string_view& str) {
    if (str == "-") {
        return UNDEFINED_COORD;
    }
    Q_ASSERT(IsCoordStringValid(str));
    return ((str[1] - '1') << BOARD_SIDE_LOG) + str[0] - 'a';
}

inline std::string CastCoordToString(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    if (c == UNDEFINED_COORD) {
        return "-";
    }
    return std::string(
        {static_cast<char>(GetYSubcoord(c) + 'a'), static_cast<char>(GetXSubcoord(c) + '1')});
}

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_BOARD_UTIL_H
