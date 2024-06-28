#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include <array>

#include "feature.h"
#include "score.h"

namespace q_eval {

inline constexpr void AssignFeatureValue(std::array<ScorePair, q_eval::FEATURE_COUNT>& res,
                                         Feature feature, score_t weight_first,
                                         score_t weight_second) {
    res[static_cast<uint16_t>(feature)] = ScorePair(weight_first, weight_second);
}

inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> GetModelWeights() {
    std::array<ScorePair, q_eval::FEATURE_COUNT> res{};
    AssignFeatureValue(res, Feature::IsolatedPawn, -5, -5);
    AssignFeatureValue(res, Feature::DoubledPawn, -30, -30);
    AssignFeatureValue(res, Feature::NoPawns, -50, -50);
    AssignFeatureValue(res, Feature::BishopPair, 32, 32);
    AssignFeatureValue(res, Feature::RookOnOpenFile, 2, 2);
    AssignFeatureValue(res, Feature::RookOnHalfOpenFile, 21, 21);
    return res;
}

static inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> MODEL_WEIGHTS =
    GetModelWeights();

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
