#ifndef QUIRKY_SRC_EVAL_SCORE_H
#define QUIRKY_SRC_EVAL_SCORE_H

#include <cstdint>
#include <cstdlib>

namespace q_eval {

using stage_t = uint8_t;
using score_t = int16_t;

constexpr score_t SCORE_MIN = -30000;
constexpr score_t SCORE_MAX = 30000;
constexpr score_t SCORE_UNKNOWN = 31000;

constexpr score_t SCORE_MATE = SCORE_MIN + 1;
constexpr score_t SCORE_ALMOST_MATE = -25000;

constexpr stage_t STAGE_MAX = 24;

inline constexpr bool IsScoreMate(const score_t score) {
    return std::abs(score) > -SCORE_ALMOST_MATE;
}

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_SCORE_H
