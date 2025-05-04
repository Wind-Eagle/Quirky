#include "model.h"

#include <cmath>
#include <cstdint>

#include "../core/util.h"
#include "core/board/geometry.h"
#include "core/board/types.h"
#include "layers.h"

namespace q_eval {

static constexpr uint16_t INPUT_LAYER_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2;
static constexpr uint16_t FEATURE_LAYER_SIZE = 64;

struct LayerStorage {
    LayerStorage() {
        ModelReader reader;
        feature_layer.Initialize(reader);
        output_layer.Initialize(reader);
    }

    FeatureLayer<INPUT_LAYER_SIZE, MODEL_INPUT_SIZE / 2> feature_layer;
    OutputLayer<MODEL_INPUT_SIZE> output_layer;
};

static LayerStorage layer_storage{};

void InitializeModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input) {
    layer_storage.feature_layer.GetResultOnEmptyBoard(input.data());
    layer_storage.feature_layer.GetResultOnEmptyBoard(input.data() + MODEL_INPUT_SIZE / 2);
}

void UpdateModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input, const q_core::cell_t cell,
                      const q_core::coord_t coord, const int8_t delta) {
    Q_ASSERT(q_core::IsCellValid(cell));
    Q_ASSERT(q_core::IsCoordValidAndDefined(coord));
    if (cell == q_core::EMPTY_CELL) {
        return;
    }
    const size_t pos_first = (static_cast<size_t>(cell) - 1) * q_core::BOARD_SIZE + coord;
    const size_t pos_second =
        (static_cast<size_t>(q_core::FlipCellColor(cell)) - 1) * q_core::BOARD_SIZE +
        q_core::FlipCoord(coord);
    layer_storage.feature_layer.Update(input.data(), pos_first, delta);
    layer_storage.feature_layer.Update(input.data() + MODEL_INPUT_SIZE / 2, pos_second, delta);
}

score_t ApplyModel(const std::array<int16_t, MODEL_INPUT_SIZE>& input, q_core::Color move_side) {
    alignas(32) std::array<int8_t, FEATURE_LAYER_SIZE> clamped_input{};
    if (move_side == q_core::Color::White) {
        for (uint16_t i = 0; i < MODEL_INPUT_SIZE; i++) {
            clamped_input[i] = std::min(std::max(input[i], static_cast<int16_t>(0)),
                                        static_cast<int16_t>(ACTIVATION_SCALE));
        }
    } else {
        for (uint16_t i = 0; i < MODEL_INPUT_SIZE / 2; i++) {
            clamped_input[i] =
                std::min(std::max(input[i + MODEL_INPUT_SIZE / 2], static_cast<int16_t>(0)),
                         static_cast<int16_t>(ACTIVATION_SCALE));
        }
        for (uint16_t i = 0; i < MODEL_INPUT_SIZE / 2; i++) {
            clamped_input[i + MODEL_INPUT_SIZE / 2] =
                std::min(std::max(input[i], static_cast<int16_t>(0)),
                         static_cast<int16_t>(ACTIVATION_SCALE));
        }
    }
    int32_t ans = layer_storage.output_layer.Process(clamped_input.data());
    return ans / WEIGHT_SCALE;
}

}  // namespace q_eval
