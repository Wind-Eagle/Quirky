#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

#include <immintrin.h>

#include <bit>

#include "macro.h"

namespace q_util {

inline constexpr void SetBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num |= (1ULL << bit);
}

inline constexpr void SetBit(uint8_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 8);
    num |= (1ULL << bit);
}

inline constexpr void FlipBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num ^= (1ULL << bit);
}

inline constexpr void ClearBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num &= ~(1ULL << bit);
}

inline constexpr bool CheckBit(const uint64_t num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    return num & (1ULL << bit);
}

inline constexpr uint8_t GetLowestBit(const uint64_t num) {
    Q_ASSERT(num > 0);
    return __builtin_ctzll(num);
}

inline constexpr uint8_t GetHighestBit(const uint64_t num) {
    Q_ASSERT(num + 1 > 0);
    return 63 - __builtin_clzll(num);
}

inline constexpr uint8_t ExtractLowestBit(uint64_t& num) {
    Q_ASSERT(num > 0);
    uint8_t ans = GetLowestBit(num);
    num &= (num - 1);
    return ans;
}

inline constexpr uint8_t GetBitCount(const uint64_t num) { return __builtin_popcountll(num); }

template <int8_t delta>
inline constexpr uint64_t MoveAllBitsByDelta(const uint64_t num) {
    if constexpr (delta > 0) {
        Q_ASSERT(num == 0 || q_util::GetHighestBit(num) + delta < 64);
        return num << delta;
    } else {
        Q_ASSERT(num == 0 || q_util::GetLowestBit(num) + delta >= 0);
        return num >> (-delta);
    }
}

inline constexpr bool ContainsBits(uint64_t num, uint64_t mask) {
    return (num & mask) == num;
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
