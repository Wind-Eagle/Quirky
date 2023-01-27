#ifndef QUIRKY_SRC_UTIL_TYPE_OPERATIONS_H
#define QUIRKY_SRC_UTIL_TYPE_OPERATIONS_H

#include "types.h"

namespace {

constexpr int8_t COLOR_OFFSET = 6;

}  // namespace

namespace util {

inline constexpr core::cell_t MakeCell(const core::Color c, const core::Piece p) {
    return static_cast<core::cell_t>(static_cast<int8_t>(p) +
                                     (c == core::Color::White ? 0 : COLOR_OFFSET));
}

inline constexpr bool IsAnyCastling(const core::Castling cst) {
    return cst != core::Castling::None;
}

template <core::Color c>
inline constexpr bool IsSideHasCastling(const core::Castling cst) {
    if constexpr (c == core::Color::White) {
        return IsAnyCastling(cst &
                             (core::Castling::WhiteKingSide | core::Castling::WhiteQueenSide));
    }
    return IsAnyCastling(cst & (core::Castling::BlackKingSide | core::Castling::BlackQueenSide));
}

inline constexpr core::Color GetCellColor(const core::cell_t cell) {
    return cell <= COLOR_OFFSET ? core::Color::White : core::Color::Black;
}

inline constexpr core::Piece GetCellPiece(const core::cell_t cell) {
    return cell <= COLOR_OFFSET ? static_cast<core::Piece>(static_cast<int>(cell))
                                : static_cast<core::Piece>(static_cast<int>(cell - COLOR_OFFSET));
}

}  // namespace util

#endif  // QUIRKY_SRC_UTIL_TYPE_OPERATIONS_H
