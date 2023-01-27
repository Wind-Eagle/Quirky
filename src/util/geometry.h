#ifndef QUIRKY_SRC_TYPES_OPERATIONS_H
#define QUIRKY_SRC_TYPES_OPERATIONS_H

namespace util {

inline constexpr core::coord_t MakeCoord(const core::subcoord_t x, const core::subcoord_t y) {
    return (x << 3) + y;
}

inline constexpr core::coord_t InvertCoord(const core::subcoord_t x) {
    return core::BOARD_SIDE - x - 1;
}

}  // namespace util

#endif  // QUIRKY_SRC_TYPES_OPERATIONS_H
