#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include <array>
#include <span>
#include <vector>

#include "psq.h"
#include "score.h"

namespace q_eval {

enum class Feature : uint16_t {
    // Simple features
    // Array features
    IsolatedPawn = 0,
    DoubledPawn = 4,
    PassedPawn = 8,
    DefendedPassedPawn = 28,
    Count = 52
};

constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

inline constexpr void AssignFeatureValue(std::array<ScorePair, q_eval::FEATURE_COUNT>& weights,
                                         std::array<uint8_t, q_eval::FEATURE_COUNT>& sizes,
                                         Feature feature, score_t weight_first,
                                         score_t weight_second) {
    weights[static_cast<uint16_t>(feature)] = ScorePair(weight_first, weight_second);
    sizes[static_cast<uint16_t>(feature)] = 1;
}

template <size_t SIZE>
inline constexpr void AssignFeatureValues(std::array<ScorePair, q_eval::FEATURE_COUNT>& weights,
                                          std::array<uint8_t, q_eval::FEATURE_COUNT>& sizes,
                                          Feature first_feature,
                                          const std::array<ScorePair, SIZE>& score_pairs) {
    for (uint16_t num = 0; num < SIZE; num++) {
        weights[static_cast<uint16_t>(first_feature) + num] = score_pairs[num];
    }
    sizes[static_cast<uint16_t>(first_feature)] = SIZE;
}

inline constexpr std::pair<std::array<ScorePair, q_eval::FEATURE_COUNT>,
                           std::array<uint8_t, q_eval::FEATURE_COUNT>>
GetModelWeightsAndFeatureSizes() {
    std::array<ScorePair, q_eval::FEATURE_COUNT> weights{};
    std::array<uint8_t, q_eval::FEATURE_COUNT> sizes{};
    AssignFeatureValues<4>(
        weights, sizes, Feature::IsolatedPawn,
        {ScorePair(-11, 3), ScorePair(-17, -9), ScorePair(-20, -3), ScorePair(-29, -3)});
    AssignFeatureValues<4>(
        weights, sizes, Feature::DoubledPawn,
        {ScorePair(8, -32), ScorePair(-2, -16), ScorePair(-5, -19), ScorePair(-3, -24)});
    AssignFeatureValues<20>(
        weights, sizes, Feature::PassedPawn,
        {ScorePair(4, -4),  ScorePair(-5, 14),  ScorePair(-13, -5), ScorePair(-24, -4),
         ScorePair(11, 2),  ScorePair(13, 5),   ScorePair(-6, 3),   ScorePair(-23, -4),
         ScorePair(18, 28), ScorePair(-23, 33), ScorePair(-21, 20), ScorePair(-14, 12),
         ScorePair(0, 59),  ScorePair(10, 57),  ScorePair(12, 38),  ScorePair(7, 37),
         ScorePair(2, 133), ScorePair(14, 117), ScorePair(0, 103),  ScorePair(-11, 78)});
    AssignFeatureValues<24>(
        weights, sizes, Feature::DefendedPassedPawn,
        {ScorePair(-6, 5),   ScorePair(18, -6),  ScorePair(39, -2), ScorePair(1, 15),
         ScorePair(4, 0),    ScorePair(-5, 9),   ScorePair(17, 3),  ScorePair(-3, 26),
         ScorePair(35, -2),  ScorePair(5, 8),    ScorePair(34, 10), ScorePair(-8, 30),
         ScorePair(60, -16), ScorePair(56, 11),  ScorePair(102, 9), ScorePair(50, 15),
         ScorePair(25, 26),  ScorePair(90, -17), ScorePair(42, 46), ScorePair(51, 6),
         ScorePair(3, 75),   ScorePair(16, 109), ScorePair(74, 88), ScorePair(22, 47)});
    return std::make_pair(weights, sizes);
}

EVAL_CONSTS_TYPE std::array<ScorePair, q_eval::FEATURE_COUNT> MODEL_WEIGHTS =
    GetModelWeightsAndFeatureSizes().first;

EVAL_CONSTS_TYPE std::array<uint8_t, q_eval::FEATURE_COUNT> MODEL_FEATURE_SIZES =
    GetModelWeightsAndFeatureSizes().second;

inline constexpr uint8_t GetModelFeatureSize(const Feature feature) {
    return MODEL_FEATURE_SIZES[static_cast<uint16_t>(feature)];
}

inline constexpr ScorePair GetModelWeight(const Feature feature, uint8_t array_index = 0) {
    Q_ASSERT(array_index < GetModelFeatureSize(feature));
    return MODEL_WEIGHTS[static_cast<uint16_t>(feature) + array_index];
}

inline constexpr ScorePair ApplyModel(const int16_t* features, size_t count) {
    Q_ASSERT(count <= FEATURE_COUNT);
    ScorePair ans{};
    for (size_t i = 0; i < count; i++) {
        ans += MODEL_WEIGHTS[i] * features[i];
    }
    return ans;
}

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
