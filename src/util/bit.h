#ifndef QUIRKY_SRC_UTIL_BIT_H
#define QUIRKY_SRC_UTIL_BIT_H

#include "macro.h"

namespace q_util {

void SetBit(uint64_t& num, const uint8_t bit) {
    Q_ASSERT(bit < 64);
    num |= (1ULL << bit);
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_BIT_H
