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
    // Simple features
    AssignFeatureValue(weights, sizes, Feature::IsolatedPawn, -17, -9);
    AssignFeatureValue(weights, sizes, Feature::DoubledPawn, -10, -40);
    AssignFeatureValue(weights, sizes, Feature::PassedPawn, -4, 29);
    AssignFeatureValue(weights, sizes, Feature::ConnectedPawn, 6, 23);
    AssignFeatureValue(weights, sizes, Feature::ConnectedPassedPawn, 11, -19);
    AssignFeatureValue(weights, sizes, Feature::NoPawns, 179, -48);
    AssignFeatureValue(weights, sizes, Feature::BishopPair, 9, 101);
    AssignFeatureValue(weights, sizes, Feature::RookOnOpenFile, 43, -28);
    AssignFeatureValue(weights, sizes, Feature::RookOnHalfOpenFile, 28, 27);

    // Array features
    AssignFeatureValues<3>(weights, sizes, Feature::PassedPawnAdvance,
                           {ScorePair(5, 26), ScorePair(31, 51), ScorePair(70, 114)});
    AssignFeatureValues<4>(
        weights, sizes, Feature::ConnectedPawnAdvance,
        {ScorePair(12, 2), ScorePair(11, 16), ScorePair(96, 42), ScorePair(50, 63)});
    AssignFeatureValues<4>(
        weights, sizes, Feature::ConnectedPassedPawnAdvance,
        {ScorePair(-19, 24), ScorePair(0, 26), ScorePair(-16, 11), ScorePair(50, 63)});
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
