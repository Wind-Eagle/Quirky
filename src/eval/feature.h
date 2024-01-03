#ifndef QUIRKY_SRC_EVAL_FEATURES_FEATURE_H
#define QUIRKY_SRC_EVAL_FEATURES_FEATURE_H

#include <cstdint>

namespace q_eval {

enum class Feature : uint16_t {
    IsolatedPawn = 0,
    DoubledPawn = 1,
    NoPawns = 2,
    BishopPair = 3,
    RookOnOpenFile = 4,
    RookOnHalfOpenFile = 5,
    Count = 6
};

constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_FEATURES_FEATURE_H
