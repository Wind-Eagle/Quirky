#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include <array>
#include <span>
#include <vector>

#include "score.h"

namespace q_eval {

enum class Feature : uint16_t {
    // Simple features
    IsolatedPawn = 0,
    DoubledPawn = 1,
    PassedPawn = 2,
    ConnectedPassedPawn = 3,
    NoPawns = 4,
    BishopPair = 5,
    RookOnOpenFile = 6,
    RookOnHalfOpenFile = 7,
    // Array features
    PassedPawnAdvance = 8,
    ConnectedPassedPawnAdvance = 11,
    Count = 15
};

constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

ScorePair GetModelWeight(Feature feature, uint8_t array_index = 0);
uint8_t GetModelFeatureSize(Feature feature);

ScorePair ApplyModel(const int16_t* features, size_t count);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
