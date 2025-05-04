#ifndef QUIRKY_SRC_CORE_BOARD_UTIL_H
#define QUIRKY_SRC_CORE_BOARD_UTIL_H

#include <string>

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

inline constexpr Color GetInvertedColor(const Color c) {
    Q_ASSERT(IsColorValid(c));
    return c == Color::White ? Color::Black : Color::White;
}

inline constexpr Color GetCellColor(const cell_t c) {
    Q_ASSERT(IsCellValid(c) && IsCellWithPiece(c));
    return c <= COLOR_OFFSET ? Color::White : Color::Black;
}

inline constexpr cell_t FlipCellColor(const cell_t c) {
    Q_ASSERT(IsCellValid(c) && IsCellWithPiece(c));
    return c <= COLOR_OFFSET ? c + COLOR_OFFSET : c - COLOR_OFFSET;
}

inline constexpr bool IsAnyCastlingAllowed(const Castling c) {
    Q_ASSERT(IsCastlingValid(c));
    return c != Castling::None;
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
    return std::string({static_cast<char>(GetFile(c) + 'a'), static_cast<char>(GetRank(c) + '1')});
}

inline constexpr Piece CastCharToPiece(char c) {
    switch (c) {
        case 'p':
            return Piece::Pawn;
        case 'n':
            return Piece::Knight;
        case 'b':
            return Piece::Bishop;
        case 'r':
            return Piece::Rook;
        case 'q':
            return Piece::Queen;
        case 'k':
            return Piece::King;
        default:
            Q_UNREACHABLE();
    }
}

inline constexpr char CastPieceToChar(Piece p) {
    Q_ASSERT(IsPieceValid(p));
    switch (p) {
        case Piece::Pawn:
            return 'p';
        case Piece::Knight:
            return 'n';
        case Piece::Bishop:
            return 'b';
        case Piece::Rook:
            return 'r';
        case Piece::Queen:
            return 'q';
        case Piece::King:
            return 'k';
        default:
            Q_UNREACHABLE();
    }
}

inline constexpr Piece GetCellPiece(const cell_t cell) {
    Q_ASSERT(IsCellValid(cell));
    Q_ASSERT(cell > 0);
    Q_ASSERT(cell < NUMBER_OF_CELLS);
    Q_ASSUME(cell < NUMBER_OF_CELLS);
    Piece ans;
    if (cell > static_cast<int8_t>(Piece::King)) {
        ans = static_cast<Piece>(cell - static_cast<int8_t>(Piece::King));
    } else {
        ans = static_cast<Piece>(cell);
    }
    return ans;
}

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_BOARD_UTIL_H
