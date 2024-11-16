#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include <array>

#include "../core/board/board.h"
#include "score.h"
#include "../util/heaparray.h"

namespace q_eval {

static constexpr uint16_t FEATURE_LAYER_SIZE = 1 * 2;

void UpdateFeatureLayer(std::array<int16_t, FEATURE_LAYER_SIZE>& feature_layer, q_core::cell_t cell, q_core::coord_t coord, int8_t delta);
score_t ApplyModel(const std::array<int16_t, FEATURE_LAYER_SIZE>& feature_layer, stage_t stage);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
