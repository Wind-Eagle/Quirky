#ifndef QUIRKY_SRC_EVAL_SCORE_H
#define QUIRKY_SRC_EVAL_SCORE_H

#include <limits>

namespace q_eval {

using stage_t = uint8_t;
using score_t = int16_t;

constexpr score_t SCORE_MIN = -30000;
constexpr score_t SCORE_MAX = 30000;

struct ScorePair {
  public:
    ScorePair() {}
    explicit ScorePair(const ScorePair& lhs) : value_(lhs.value_) {}
    ScorePair(const score_t first, const score_t second) : value_(first + second * (1 << 16)) {}
    explicit ScorePair(const score_t score) : ScorePair(score, score) {}

    score_t GetFirst() const {
        return static_cast<score_t>(value_);
    }
    score_t GetSecond() const {
        score_t ans = value_ >> 16;
        if (GetFirst() < 0) {
            ans++;
        }
        return ans;
    }

    score_t GetEvaluationInStage(const stage_t stage) const {
        return GetFirst() * stage + GetSecond() * (std::numeric_limits<stage_t>::max() - stage);
    }
  private:
    int32_t value_ = 0;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_SCORE_H
