#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

#ifndef NO_BMI2
#include <immintrin.h>
#endif

#include <cstdint>
#include <concepts>

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

#ifndef NO_BMI2
inline uint64_t DepositBits(const uint64_t submask, const uint64_t mask) {
    return _pdep_u64(submask, mask);
}

inline uint64_t ExtractBits(const uint64_t mask, const uint64_t submask) {
    return _pext_u64(mask, submask);
}
#else
inline uint64_t DepositBits(uint64_t src, uint64_t mask) {
    uint64_t result = 0;
    int src_pos = 0;
    while (mask) {
        const uint64_t lsb = mask & -mask;
        if (src & (1ULL << src_pos)) {
            result |= lsb;
        }
        src_pos++;
        mask ^= lsb;
    }
    return result;
}
inline uint64_t ExtractBits(uint64_t src, uint64_t mask) {
    uint64_t result = 0;
    int res_pos = 0;
    while (mask) {
        const uint64_t lsb = mask & -mask;
        if (src & lsb) {
            result |= (1ULL << res_pos);
        }
        res_pos++;
        mask ^= lsb;
    }
    return result;
}
#endif

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_BIT_H
