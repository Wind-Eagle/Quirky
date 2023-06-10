#ifndef QUIRKY_SRC_CORE_BOARD_GEOMETRY_H
#define QUIRKY_SRC_CORE_BOARD_GEOMETRY_H

#include "types.h"

namespace q_core {

inline constexpr subcoord_t MakeCoord(const subcoord_t x, const subcoord_t y) {
    Q_ASSERT(IsSubcoordValid(x) && IsSubcoordValid(y));
    return (x << BOARD_SIDE_LOG) | y;
}

inline constexpr subcoord_t InvertSubcoord(const subcoord_t c) {
    Q_ASSERT(IsSubcoordValid(c));
    return BOARD_SIDE - 1 - c;
}

inline constexpr subcoord_t GetXSubcoord(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    return c >> BOARD_SIDE_LOG;
}

inline constexpr subcoord_t GetYSubcoord(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    return c & ((1 << BOARD_SIDE_LOG) - 1);
}

}  // namespace

#endif  // QUIRKY_SRC_CORE_BOARD_GEOMETRY_H
