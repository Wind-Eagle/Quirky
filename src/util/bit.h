#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

#include <immintrin.h>

#include <bit>
#include <concepts>
#include <limits>

#include "macro.h"

namespace q_util {

inline constexpr void SetBit(std::unsigned_integral auto& num, const uint8_t bit) {
    Q_ASSERT(bit < std::numeric_limits<std::decay_t<decltype(num)>>::digits);
    num |= (1ULL << bit);
}

inline constexpr void FlipBit(std::unsigned_integral auto& num, const uint8_t bit) {
    Q_ASSERT(bit < std::numeric_limits<std::decay_t<decltype(num)>>::digits);
    num ^= (1ULL << bit);
}

inline constexpr void ClearBit(std::unsigned_integral auto& num, const uint8_t bit) {
    Q_ASSERT(bit < std::numeric_limits<std::decay_t<decltype(num)>>::digits);
    num &= ~(1ULL << bit);
}

inline constexpr bool CheckBit(const std::unsigned_integral auto num, const uint8_t bit) {
    Q_ASSERT(bit < std::numeric_limits<decltype(num)>::digits);
    return num & (1ULL << bit);
}

inline constexpr uint8_t GetLowestBit(const std::unsigned_integral auto num) {
    Q_ASSERT(num > 0);
    return __builtin_ctzll(num);
}

inline constexpr uint8_t GetHighestBit(const std::unsigned_integral auto num) {
    Q_ASSERT(num + 1 > 0);
    return 63 - __builtin_clzll(num);
}

inline constexpr uint8_t ExtractLowestBit(std::unsigned_integral auto& num) {
    Q_ASSERT(num > 0);
    uint8_t ans = GetLowestBit(num);
    num &= (num - 1);
    return ans;
}

inline constexpr void ClearBits(std::unsigned_integral auto& num, const uint64_t bits) {
    num &= ~(bits);
}

inline constexpr uint8_t GetBitCount(const std::unsigned_integral auto num) {
    return __builtin_popcountll(num);
}

template <int8_t delta, std::unsigned_integral T>
inline constexpr T MoveAllBitsByDelta(const T num) {
    if constexpr (delta > 0) {
        Q_ASSERT(num == 0 ||
                 q_util::GetHighestBit(num) + delta < std::numeric_limits<decltype(num)>::digits);
        return num << delta;
    } else {
        Q_ASSERT(num == 0 || q_util::GetLowestBit(num) + delta >= 0);
        return num >> (-delta);
    }
}

inline constexpr uint64_t ScatterByte(const uint8_t num) {
    uint64_t ans = num;
    ans |= ans << 8;
    ans |= ans << 16;
    ans |= ans << 32;
    return ans;
}

inline uint64_t DepositBits(const uint64_t submask, const uint64_t mask) {
    return _pdep_u64(submask, mask);
}

inline uint64_t ExtractBits(const uint64_t mask, const uint64_t submask) {
    return _pext_u64(mask, submask);
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_BIT_H
