#ifndef QUIRKY_SRC_UTIL_DISTANCE_H
#define QUIRKY_SRC_UTIL_DISTANCE_H

#include <algorithm>

namespace q_util {

inline constexpr auto Abs(std::signed_integral auto num) {
    return num >= 0 ? num : -num;
}

inline constexpr uint8_t GetL1Distance(int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    return Abs(x1 - x2) + Abs(y1 - y2);
}

inline constexpr uint8_t GetLInftyDistance(int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    return std::max(Abs(x1 - x2), Abs(y1 - y2));
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_DISTANCE_H
