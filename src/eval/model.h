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
    NoPawns = 2,
    BishopPair = 3,
    RookOnOpenFile = 4,
    RookOnHalfOpenFile = 5,
    // Array features
    QueensidePawnShield = 6,
    QueensidePawnStorm = 12,
    KingsidePawnShield = 18,
    KingsidePawnStorm = 24,
    QueenDistanceToKing = 30,
    Count = 38
};

constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

ScorePair GetModelWeight(Feature feature, uint8_t array_index = 0);
uint8_t GetModelFeatureSize(Feature feature);

ScorePair ApplyModel(const int16_t* features, size_t count);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
