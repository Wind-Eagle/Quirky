#include "model.h"

#include "../core/util.h"

#include <fstream>

namespace q_eval {

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct FeatureLayer {
    std::array<std::array<float, OUTPUT_SIZE>, INPUT_SIZE> weights;
    std::array<float, OUTPUT_SIZE> biases;

    void Update(float* input, size_t position, int8_t delta) {
        for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
            input[i] += weights[position][i] * delta;
        }
    }
};

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE, bool APPLY_RELU>
struct LinearLayer {
    std::array<std::array<float, OUTPUT_SIZE>, INPUT_SIZE> weights;
    std::array<float, OUTPUT_SIZE> biases;

    void Process(const float* input, float* output) {
        std::copy(biases.begin(), biases.end(), output);
        for (uint16_t i = 0; i < INPUT_SIZE; i++) {
            for (uint16_t j = 0; j < OUTPUT_SIZE; j++) {
                output[j] += weights[i][j] * input[i];
            }
        }
        if constexpr (APPLY_RELU) {
            for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
                output[i] = std::min(std::max(output[i], static_cast<float>(0.0)), static_cast<float>(1.0));
            }
        }
    }
};

static constexpr uint16_t INPUT_LAYER_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2;
static constexpr uint16_t FEATURE_LAYER_SIZE = 8;
static constexpr uint16_t HIDDEN_LAYER_SIZE = 8;

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
void ReadWeights(std::array<std::array<float, OUTPUT_SIZE>, INPUT_SIZE>& weights, std::fstream& in) {
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        for (size_t j = 0; j < OUTPUT_SIZE; j++) {
            in >> weights[i][j];
        }
    }
}

template <size_t INPUT_SIZE>
void ReadBiases(std::array<float, INPUT_SIZE>& biases, std::fstream& in) {
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        in >> biases[i];
    }
}

struct LayerStorage {
    LayerStorage() {
        std::fstream in("/home/wind-eagle/Quirky/build/model.qnne");
        ReadWeights(feature_layer.weights, in);
        ReadWeights(hidden_layer.weights, in);
        ReadWeights(output_layer.weights, in);
        ReadBiases(feature_layer.biases, in);
        ReadBiases(hidden_layer.biases, in);
        ReadBiases(output_layer.biases, in);
    }

    FeatureLayer<INPUT_LAYER_SIZE, FEATURE_LAYER_SIZE> feature_layer;
    LinearLayer<FEATURE_LAYER_SIZE, HIDDEN_LAYER_SIZE, true> hidden_layer;
    LinearLayer<HIDDEN_LAYER_SIZE, 1, false> output_layer;
};

static LayerStorage layer_storage{};

void InitializeModelInput(std::array<float, MODEL_INPUT_SIZE>& input) {
    std::copy(layer_storage.feature_layer.biases.begin(),
              layer_storage.feature_layer.biases.end(), input.begin());
}

void UpdateModelInput(std::array<float, MODEL_INPUT_SIZE>& input, const q_core::cell_t cell,
                      const q_core::coord_t coord, const int8_t delta) {
    Q_ASSERT(q_core::IsCellValid(cell));
    Q_ASSERT(q_core::IsCoordValidAndDefined(coord));
    if (cell == q_core::EMPTY_CELL) {
        return;
    }
    const size_t pos = (static_cast<size_t>(cell) - 1) * q_core::BOARD_SIZE + coord;
    layer_storage.feature_layer.Update(input.data(), pos, delta);
}

score_t ApplyModel(const std::array<float, MODEL_INPUT_SIZE>& input, stage_t) {
    std::array<float, MODEL_INPUT_SIZE> clamped_input;
    for (uint16_t i = 0; i < MODEL_INPUT_SIZE; i++) {
        clamped_input[i] = std::min(std::max(input[i], static_cast<float>(0.0)), static_cast<float>(1.0));
    }
    std::array<float, HIDDEN_LAYER_SIZE + 1> buffer;
    layer_storage.hidden_layer.Process(clamped_input.data(), buffer.data());
    layer_storage.output_layer.Process(buffer.data(), buffer.data() + HIDDEN_LAYER_SIZE);
    return buffer[HIDDEN_LAYER_SIZE] * 80;
}

}  // namespace q_eval
