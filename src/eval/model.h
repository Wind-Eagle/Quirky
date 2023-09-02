#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include "feature.h"
#include "score.h"

#include <array>

namespace q_eval {

inline constexpr void AssignFeatureValue(std::array<ScorePair, q_eval::FEATURE_COUNT>& res, Feature feature, ScorePair weight) {
    res[static_cast<uint16_t>(feature)] = weight;
}

inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> GetModelWeights() {
    std::array<ScorePair, q_eval::FEATURE_COUNT> res{};
    return res;
}

inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> MODEL_WEIGHTS = GetModelWeights();

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
