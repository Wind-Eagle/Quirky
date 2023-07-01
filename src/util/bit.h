#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

#include "macro.h"

#include <bit>

namespace q_util {

inline constexpr void SetBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
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
    return std::countr_zero(num);
}

inline constexpr uint8_t GetHighestBit(const uint64_t num) {
    Q_ASSERT(num + 1 > 0);
    return std::countl_zero(num);
}

inline constexpr uint8_t ExtractLowestBit(uint64_t& num) {
    Q_ASSERT(num > 0);
    uint8_t ans = GetLowestBit(num);
    ClearBit(num, ans);
    return ans;
}

inline constexpr uint8_t GetBitCount(const uint64_t num) {
    return std::popcount(num);
}

template <uint8_t delta>
inline constexpr uint64_t MoveAllBitsByDelta(const uint64_t num) {
    if constexpr (delta > 0) {
        Q_ASSERT(q_util::GetHighestBit(num) + delta < 64);
        return num << delta;
    } else {
        Q_ASSERT(q_util::GetLowestBit(num) + delta >= 0);
        return num >> (-delta);
    }
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_BIT_H
