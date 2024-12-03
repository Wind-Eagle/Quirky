#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include <array>

#include "../core/board/board.h"
#include "score.h"
#include "../util/heaparray.h"

namespace q_eval {

static constexpr uint16_t MODEL_INPUT_SIZE = 16;

void InitializeModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input);
void UpdateModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input, q_core::cell_t cell, q_core::coord_t coord, int8_t delta);
score_t ApplyModel(const std::array<int16_t, MODEL_INPUT_SIZE>& input);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
