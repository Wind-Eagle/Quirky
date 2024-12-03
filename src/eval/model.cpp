#include "model.h"

#include "../core/util.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <type_traits>

namespace q_eval {

static constexpr uint16_t INPUT_LAYER_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2;
static constexpr uint16_t FEATURE_LAYER_SIZE = 16;
static constexpr uint16_t HIDDEN_LAYER_SIZE = 8;

template <std::integral T>
T ReadWeight(std::fstream& in, int scale) {
    float weight;
    in >> weight;
    int64_t final_weight = std::round(weight * scale);
    final_weight = std::min(final_weight, static_cast<int64_t>(std::numeric_limits<T>::max()));
    final_weight = std::max(final_weight, static_cast<int64_t>(std::numeric_limits<T>::min()));
    return final_weight;
}

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE, std::integral T>
void ReadWeights(std::array<std::array<T, OUTPUT_SIZE>, INPUT_SIZE>& weights, std::fstream& in, float scale) {
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        for (size_t j = 0; j < OUTPUT_SIZE; j++) {
            weights[i][j] = ReadWeight<T>(in, scale);
        }
    }
}

template <size_t INPUT_SIZE, std::integral T>
void ReadBiases(std::array<T, INPUT_SIZE>& biases, std::fstream& in, float scale) {
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        biases[i] = ReadWeight<T>(in, scale);
    }
}

static constexpr float WEIGHT_SCALE = 64;
static constexpr float ACTIVATION_SCALE = 127;
static constexpr float OUTPUT_SCALE = 64;

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct FeatureLayer {
    std::array<std::array<int16_t, OUTPUT_SIZE>, INPUT_SIZE> weights;
    std::array<int16_t, OUTPUT_SIZE> biases;

    void Update(int16_t* input, size_t position, int8_t delta) {
        for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
            input[i] += weights[position][i] * delta;
        }
    }
};

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE, bool APPLY_RELU>
struct LinearLayer {
    std::array<std::array<int8_t, OUTPUT_SIZE>, INPUT_SIZE> weights;
    std::array<int32_t, OUTPUT_SIZE> biases;

    void Process(const int8_t* input, int32_t* buffer, int8_t* output) {
        std::copy(biases.begin(), biases.end(), buffer);
        for (uint16_t i = 0; i < INPUT_SIZE; i++) {
            for (uint16_t j = 0; j < OUTPUT_SIZE; j++) {
                buffer[j] += static_cast<int16_t>(weights[i][j]) * input[i];
            }
        }
        if constexpr (APPLY_RELU) {
            for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
                output[i] = std::min(std::max(buffer[i] / static_cast<int32_t>(WEIGHT_SCALE), static_cast<int32_t>(0)), static_cast<int32_t>(127));
            }
        }
    }
};

struct LayerStorage {
    LayerStorage() {
        std::fstream in("/home/wind-eagle/Quirky/build/big_model2.qnne");
        ReadWeights(feature_layer.weights, in, ACTIVATION_SCALE);
        ReadWeights(hidden_layer.weights, in, WEIGHT_SCALE);
        ReadWeights(output_layer.weights, in, WEIGHT_SCALE * OUTPUT_SCALE / ACTIVATION_SCALE);
        ReadBiases(feature_layer.biases, in, ACTIVATION_SCALE);
        ReadBiases(hidden_layer.biases, in, ACTIVATION_SCALE * WEIGHT_SCALE);
        ReadBiases(output_layer.biases, in, WEIGHT_SCALE * OUTPUT_SCALE);
    }

    FeatureLayer<INPUT_LAYER_SIZE, FEATURE_LAYER_SIZE> feature_layer;
    LinearLayer<FEATURE_LAYER_SIZE, HIDDEN_LAYER_SIZE, true> hidden_layer;
    LinearLayer<HIDDEN_LAYER_SIZE, 1, false> output_layer;
};

static LayerStorage layer_storage{};

void InitializeModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input) {
    std::copy(layer_storage.feature_layer.biases.begin(),
              layer_storage.feature_layer.biases.end(), input.begin());
}

void UpdateModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input, const q_core::cell_t cell,
                      const q_core::coord_t coord, const int8_t delta) {
    Q_ASSERT(q_core::IsCellValid(cell));
    Q_ASSERT(q_core::IsCoordValidAndDefined(coord));
    if (cell == q_core::EMPTY_CELL) {
        return;
    }
    const size_t pos = (static_cast<size_t>(cell) - 1) * q_core::BOARD_SIZE + coord;
    layer_storage.feature_layer.Update(input.data(), pos, delta);
}

score_t ApplyModel(const std::array<int16_t, MODEL_INPUT_SIZE>& input, stage_t) {
    std::array<int8_t, MODEL_INPUT_SIZE> clamped_input;
    for (uint16_t i = 0; i < MODEL_INPUT_SIZE; i++) {
        clamped_input[i] = std::min(std::max(input[i], static_cast<int16_t>(0)), static_cast<int16_t>(127));
    }
    std::array<int32_t, HIDDEN_LAYER_SIZE> buffer;
    std::array<int8_t, HIDDEN_LAYER_SIZE + 1> hidden_output;
    layer_storage.hidden_layer.Process(clamped_input.data(), buffer.data(), hidden_output.data());
    layer_storage.output_layer.Process(hidden_output.data(), buffer.data(), hidden_output.data() + HIDDEN_LAYER_SIZE);
    return buffer[0] / WEIGHT_SCALE;
}

}  // namespace q_eval
