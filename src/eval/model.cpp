#include "model.h"

#include <cmath>
#include <cstdint>

#include "../core/util.h"
#include "../core/board/geometry.h"
#include "../core/board/types.h"
#include "layers.h"

namespace q_eval {

static constexpr uint16_t INPUT_LAYER_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2;
static constexpr uint16_t FEATURE_LAYER_SIZE = 256;
static constexpr uint16_t HIDDEN_LAYER_FIRST_SIZE = 16;
static constexpr uint16_t HIDDEN_LAYER_SECOND_SIZE = 32;

struct LayerStorage {
    LayerStorage() {
        ModelReader reader;
        feature_layer.Initialize(reader);
        hidden_layer_first.Initialize(reader);
        hidden_layer_second.Initialize(reader);
        output_layer.Initialize(reader);
    }

    FeatureLayer<INPUT_LAYER_SIZE, MODEL_INPUT_SIZE / 2> feature_layer;
    LinearLayer<FEATURE_LAYER_SIZE, HIDDEN_LAYER_FIRST_SIZE> hidden_layer_first;
    PreciseLinearLayer<HIDDEN_LAYER_FIRST_SIZE, HIDDEN_LAYER_SECOND_SIZE> hidden_layer_second;
    OutputLayer<HIDDEN_LAYER_SECOND_SIZE> output_layer;
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
    alignas(64) std::array<int8_t, FEATURE_LAYER_SIZE> clamped_input{};
    if (move_side == q_core::Color::White) {
        ClippedReLU16(MODEL_INPUT_SIZE, clamped_input.data(), input.data());
    } else {
        ClippedReLU16(MODEL_INPUT_SIZE / 2, clamped_input.data(), input.data() + MODEL_INPUT_SIZE / 2);
        ClippedReLU16(MODEL_INPUT_SIZE / 2, clamped_input.data() + MODEL_INPUT_SIZE / 2, input.data());
    }

    alignas(64) std::array<int32_t, HIDDEN_LAYER_SECOND_SIZE> buffer;
    alignas(64) std::array<int16_t, HIDDEN_LAYER_FIRST_SIZE> hidden_output_first{};
    layer_storage.hidden_layer_first.Process(clamped_input.data(), buffer.data());
    ClippedReLU32(HIDDEN_LAYER_FIRST_SIZE, hidden_output_first.data(), buffer.data());

    alignas(64) std::array<int16_t, HIDDEN_LAYER_SECOND_SIZE> hidden_output_second{};
    layer_storage.hidden_layer_second.Process(hidden_output_first.data(), buffer.data());
    ClippedReLU32(HIDDEN_LAYER_SECOND_SIZE, hidden_output_second.data(), buffer.data());

    int32_t ans = layer_storage.output_layer.Process(hidden_output_second.data());
    return ans / OUTPUT_SCALE / WEIGHT_SCALE;
}

}  // namespace q_eval
