#ifndef QUIRKY_SRC_UTIL_DISTANCE_H
#define QUIRKY_SRC_UTIL_DISTANCE_H

#include <cstdlib>

namespace q_util {

uint8_t GetL1Distance(int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

uint8_t GetLInftyDistance(int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    return std::max(std::abs(x1 - x2), std::abs(y1 - y2));
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_DISTANCE_H
