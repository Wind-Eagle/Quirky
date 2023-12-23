#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include "feature.h"
#include "score.h"

#include <array>

namespace q_eval {

inline constexpr void AssignFeatureValue(std::array<ScorePair, q_eval::FEATURE_COUNT>& res, Feature feature, score_t weight_first, score_t weight_second) {
    res[static_cast<uint16_t>(feature)] = ScorePair(weight_first, weight_second);
}

inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> GetModelWeights() {
    std::array<ScorePair, q_eval::FEATURE_COUNT> res{};
    AssignFeatureValue(res, Feature::IsolatedPawn, -16, -16);
    AssignFeatureValue(res, Feature::DoubledPawn, -17, -17);
    AssignFeatureValue(res, Feature::PawnIslands, 0, -25);
    AssignFeatureValue(res, Feature::NoPawns, -50, -50);
    AssignFeatureValue(res, Feature::BishopPair, 0, 20);
    AssignFeatureValue(res, Feature::RookOnOpenFile, 35, 0);
    AssignFeatureValue(res, Feature::RookOnHalfOpenFile, 17, 0);
    return res;
}

static inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> MODEL_WEIGHTS = GetModelWeights();

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
