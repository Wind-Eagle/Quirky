#include "model.h"
#include "layers.h"

#include "../core/util.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <type_traits>

namespace q_eval {

static constexpr uint16_t INPUT_LAYER_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2;
static constexpr uint16_t FEATURE_LAYER_SIZE = 32;
static constexpr uint16_t HIDDEN_LAYER_SIZE = 8;

struct LayerStorage {
    LayerStorage() {
        ModelReader reader;
        feature_layer.Initialize(reader);
        hidden_layer.Initialize(reader);
        output_layer.Initialize(reader);
    }

    FeatureLayer<INPUT_LAYER_SIZE, MODEL_INPUT_SIZE> feature_layer;
    LinearLayer<FEATURE_LAYER_SIZE, HIDDEN_LAYER_SIZE> hidden_layer;
    OutputLayer<HIDDEN_LAYER_SIZE> output_layer;
};

static LayerStorage layer_storage{};

void InitializeModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input) {
    layer_storage.feature_layer.GetResultOnEmptyBoard(input);
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

score_t ApplyModel(const std::array<int16_t, MODEL_INPUT_SIZE>& input) {
    alignas(32) std::array<int8_t, FEATURE_LAYER_SIZE> clamped_input{};
    for (uint16_t i = 0; i < MODEL_INPUT_SIZE; i++) {
        clamped_input[i] = std::min(std::max(input[i], static_cast<int16_t>(0)), static_cast<int16_t>(ACTIVATION_SCALE));
    }
    alignas(32) std::array<int32_t, HIDDEN_LAYER_SIZE> buffer;
    alignas(32) std::array<int8_t, HIDDEN_LAYER_SIZE> hidden_output;
    layer_storage.hidden_layer.Process(clamped_input.data(), buffer.data());
    for (uint16_t i = 0; i < HIDDEN_LAYER_SIZE; i++) {
        hidden_output[i] = std::min(std::max(buffer[i] / static_cast<int32_t>(WEIGHT_SCALE), static_cast<int32_t>(0)), static_cast<int32_t>(ACTIVATION_SCALE));
    }
    int32_t ans = layer_storage.output_layer.Process(hidden_output.data());
    return ans / WEIGHT_SCALE;
}

}  // namespace q_eval
