#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

namespace util {

inline void SetBit(uint64_t& number, const uint8_t bit) {
    number |= (1ULL << bit);
}

inline void FlipBit(uint64_t& number, const uint8_t bit) {
    number ^= (1ULL << bit);
}

inline void ResetBit(uint64_t& number, const uint8_t bit) {
    number &= ~(1ULL << bit);
}

}  // namespace util

#endif  // QUIRKY_SRC_UTIL_BIT_H
