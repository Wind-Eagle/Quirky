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
    ConnectedPawn = 3,
    ConnectedPassedPawn = 4,
    NoPawns = 5,
    BishopPair = 6,
    RookOnOpenFile = 7,
    RookOnHalfOpenFile = 8,
    // Array features
    PassedPawnAdvance = 9,
    ConnectedPawnAdvance = 12,
    ConnectedPassedPawnAdvance = 16,
    Count = 20
};

constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

ScorePair GetModelWeight(Feature feature, uint8_t array_index = 0);
uint8_t GetModelFeatureSize(Feature feature);

ScorePair ApplyModel(const int16_t* features, size_t count);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
