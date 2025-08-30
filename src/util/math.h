#ifndef QUIRKY_SRC_UTIL_DISTANCE_H
#define QUIRKY_SRC_UTIL_DISTANCE_H

#include <algorithm>

namespace q_util {

inline constexpr auto Abs(std::signed_integral auto num) { return num >= 0 ? num : -num; }

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_DISTANCE_H
