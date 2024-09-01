#ifndef QUIRKY_SRC_UTIL_RANDOM_H
#define QUIRKY_SRC_UTIL_RANDOM_H

#include <cstdint>
#include <iostream>

namespace q_util {

inline uint64_t GetRandom64() {
    static uint64_t x = 1;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_RANDOM_H
