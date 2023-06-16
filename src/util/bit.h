#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

#include "macro.h"

#include <bit>

namespace q_util {

void SetBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num |= (1ULL << bit);
}

void FlipBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num ^= (1ULL << bit);
}

void ClearBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num &= ~(1ULL << bit);
}

bool CheckBit(const uint64_t num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    return num & (1ULL << bit);
}

uint8_t GetLowestBit(const uint64_t num) {
    Q_ASSERT(num > 0);
    return std::countr_zero(num);
}

uint8_t GetHighestBit(const uint64_t num) {
    Q_ASSERT(num + 1 > 0);
    return std::countl_zero(num);
}

uint8_t ExtractLowestBit(uint64_t& num) {
    Q_ASSERT(num > 0);
    uint8_t ans = GetLowestBit(num);
    ClearBit(num, ans);
    return ans;
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_BIT_H
