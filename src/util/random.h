#ifndef QUIRKY_SRC_UTIL_RANDOM_H
#define QUIRKY_SRC_UTIL_RANDOM_H

#include <array>

#include "string.h"

namespace util {

/// Generates random 64-bit number. This function is not thread-safe.
inline uint64_t GetRandom64() {
    static uint64_t x = 1;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

/// A constexpr function to generate random std::array with size N and random seed x
template <size_t N>
inline constexpr std::array<uint64_t, N> MakeRandomArray64(const char* name) {
    std::array<uint64_t, N> res;
    uint64_t x = GetStringHash(name);
    for (uint32_t i = 0; i < N; i++) {
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        x *= 0x2545F4914F6CDD1DULL;
        res[i] = x;
    }
    return res;
}

}  // namespace util

#endif  // QUIRKY_SRC_UTIL_RANDOM_H
