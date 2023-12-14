#ifndef QUIRKY_SRC_UTIL_HASH_H
#define QUIRKY_SRC_UTIL_HASH_H

#include <cstddef>
#include <cstdint>

// The code here is derived from FarmHash64 by Google:
// https://github.com/google/farmhash/blob/master/src/farmhash.cc.
//
// FarmHash is distributed under the following license terms:
//
// Copyright (c) 2014 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

namespace {

constexpr uint64_t HASH_K1 = 0xb492b66fbe98f273ULL;
constexpr uint64_t HASH_K2 = 0x9ae16a3b2f90404fULL;

inline constexpr uint64_t FinalizeHash(const uint64_t u, const uint64_t v, const uint64_t mul) {
    uint64_t a = (u ^ v) * mul;
    a ^= (a >> 47);
    uint64_t b = (v ^ a) * mul;
    b ^= (b >> 47);
    b *= mul;
    return b;
}

inline constexpr uint64_t RotateLeft(const uint64_t x, const size_t shift) {
    return (shift == 0) ? x : ((x << shift) | (x >> (64 - shift)));
}

inline constexpr uint64_t RotateRight(const uint64_t x, const size_t shift) {
    return (shift == 0) ? x : ((x >> shift) | (x << (64 - shift)));
}

inline constexpr uint64_t GetHash16(const uint64_t v0, const uint64_t v1) {
    constexpr uint64_t LEN = 16;
    const uint64_t mul = HASH_K2 + LEN * 2;
    const uint64_t a = v0 + HASH_K2;
    const uint64_t b = v1;
    const uint64_t c = RotateRight(b, 37) * mul + a;
    const uint64_t d = (RotateRight(a, 25) + b) * mul;
    return FinalizeHash(c, d, mul);
}

inline constexpr uint64_t GetHash32(const uint64_t v0, const uint64_t v1, const uint64_t v2,
                                    const uint64_t v3) {
    constexpr uint64_t LEN = 32;
    const uint64_t mul = HASH_K2 + LEN * 2;
    const uint64_t a = v0 * HASH_K1;
    const uint64_t b = v1;
    const uint64_t c = v2 * mul;
    const uint64_t d = v3 * HASH_K2;
    return FinalizeHash(RotateRight(a + b, 43) + RotateRight(c, 30) + d,
                        a + RotateRight(b + HASH_K2, 18) + c, mul);
}

}  // namespace

namespace q_util {

inline constexpr uint64_t GetStringHash(const std::string_view& str) {
    uint64_t ans = 0;
    uint64_t cur = 0;
    size_t index = 0;
    for (const auto c : str) {
        cur <<= 8;
        cur += c;
        if (!(index & 7)) {
            ans = GetHash16(ans, cur);
            cur = 0;
        }
    }
    if (index & 7) {
        ans = GetHash16(ans, cur);
    }
    return ans;
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_HASH_H
