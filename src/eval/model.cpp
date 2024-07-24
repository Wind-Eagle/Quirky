#include "model.h"
#include "score.h"

namespace q_eval {

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
    AssignFeatureValue(weights, sizes, Feature::IsolatedPawn, -5, -5);
    AssignFeatureValue(weights, sizes, Feature::DoubledPawn, -30, -30);
    AssignFeatureValue(weights, sizes, Feature::NoPawns, -50, -50);
    AssignFeatureValue(weights, sizes, Feature::BishopPair, 32, 32);
    AssignFeatureValue(weights, sizes, Feature::RookOnOpenFile, 2, 2);
    AssignFeatureValue(weights, sizes, Feature::RookOnHalfOpenFile, 21, 21);

    AssignFeatureValues<6>(weights, sizes, Feature::QueensidePawnShield,
                           {ScorePair(7, 0), ScorePair(65, 0), ScorePair(9, 0), ScorePair(-15, 0),
                            ScorePair(-56, 0), ScorePair(24, 0)});
    AssignFeatureValues<6>(weights, sizes, Feature::QueensidePawnStorm,
                           {ScorePair(49, 0), ScorePair(-23, 0), ScorePair(-78, 0), ScorePair(7, 0),
                            ScorePair(2, 0), ScorePair(22, 0)});
    AssignFeatureValues<6>(weights, sizes, Feature::KingsidePawnShield,
                           {ScorePair(9, 0), ScorePair(65, 0), ScorePair(7, 0), ScorePair(24, 0),
                            ScorePair(56, 0), ScorePair(-15, 0)});
    AssignFeatureValues<6>(weights, sizes, Feature::KingsidePawnStorm,
                           {ScorePair(-78, 0), ScorePair(-23, 0), ScorePair(49, 0), ScorePair(22, 0),
                            ScorePair(2, 0), ScorePair(7, 0)});
    return std::make_pair(weights, sizes);
}

static inline constexpr std::array<ScorePair, q_eval::FEATURE_COUNT> MODEL_WEIGHTS =
    GetModelWeightsAndFeatureSizes().first;

static inline constexpr std::array<uint8_t, q_eval::FEATURE_COUNT> MODEL_FEATURE_SIZES =
    GetModelWeightsAndFeatureSizes().second;

ScorePair GetModelWeight(const Feature feature, uint8_t array_index) {
    Q_ASSERT(array_index < GetModelFeatureSize(feature));
    return MODEL_WEIGHTS[static_cast<uint16_t>(feature) + array_index];
}

uint8_t GetModelFeatureSize(const Feature feature) {
    return MODEL_FEATURE_SIZES[static_cast<uint16_t>(feature)];
}

ScorePair ApplyModel(const int16_t* features, size_t count) {
    Q_ASSERT(count <= FEATURE_COUNT);
    ScorePair ans{};
    for (size_t i = 0; i < count; i++) {
        ans += MODEL_WEIGHTS[i] * features[i];
    }
    return ans;
}

}  // namespace q_eval
