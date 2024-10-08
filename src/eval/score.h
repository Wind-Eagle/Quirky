#ifndef QUIRKY_SRC_EVAL_SCORE_H
#define QUIRKY_SRC_EVAL_SCORE_H

#include <cstdint>
#include <limits>

#include "../util/macro.h"
#include "../util/math.h"

namespace q_eval {

using stage_t = uint8_t;
using score_t = int16_t;

constexpr score_t SCORE_MIN = -30000;
constexpr score_t SCORE_MAX = 30000;
constexpr score_t SCORE_UNKNOWN = 31000;

constexpr score_t SCORE_MATE = SCORE_MIN + 1;
constexpr score_t SCORE_ALMOST_MATE = -25000;

inline constexpr bool IsScoreMate(const score_t score) {
    return q_util::Abs(score) > -SCORE_ALMOST_MATE;
}

struct ScorePair {
  public:
    constexpr ScorePair() {}
    constexpr ScorePair(const ScorePair& lhs) : value_(lhs.value_) {}
    constexpr ScorePair(const score_t first, const score_t second)
        : value_(first + second * (1 << 16)) {}

    constexpr score_t GetFirst() const { return static_cast<score_t>(value_); }
    constexpr score_t GetSecond() const {
        score_t ans = value_ >> 16;
        if (GetFirst() < 0) {
            ans++;
        }
        return ans;
    }

    constexpr ScorePair& operator+=(const ScorePair rhs) {
        value_ += rhs.value_;
        return *this;
    }

    constexpr ScorePair& operator-=(const ScorePair rhs) {
        value_ -= rhs.value_;
        return *this;
    }

    constexpr ScorePair& operator=(const ScorePair rhs) {
        value_ = rhs.value_;
        return *this;
    }
    constexpr bool operator==(const ScorePair rhs) const { return value_ == rhs.value_; }

    constexpr ScorePair operator+(const ScorePair rhs) const {
        return ScorePair(value_ + rhs.value_);
    }
    constexpr ScorePair operator-(const ScorePair rhs) const {
        return ScorePair(value_ - rhs.value_);
    }
    constexpr ScorePair operator*(const int8_t rhs) const { return ScorePair(value_ * rhs); }
    constexpr ScorePair& operator*=(const int8_t rhs) {
        value_ *= rhs;
        return *this;
    }

  private:
    constexpr ScorePair(const int32_t lhs) : value_(lhs) {}
    constexpr ScorePair(const score_t score) : ScorePair(score, score) {}
    int32_t value_ = 0;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_SCORE_H
