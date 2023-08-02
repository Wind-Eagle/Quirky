#ifndef QUIRKY_SRC_EVAL_FEATURES_FEATURE_H
#define QUIRKY_SRC_EVAL_FEATURES_FEATURE_H

#include <cstdint>

namespace q_eval {

enum class Feature : uint16_t {
    BackwardPawn = 0,
    BackwardPawnOnHalfOpenFile = 1,
    Count = 2
};

constexpr uint16_t DYNAMIC_FEATURE_COUNT = 0;
constexpr uint16_t PAWN_FEATURE_COUNT = 0;
constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_FEATURES_FEATURE_H
