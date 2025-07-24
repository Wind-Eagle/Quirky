#ifndef QUIRKY_SRC_EVAL_MODEL_H
#define QUIRKY_SRC_EVAL_MODEL_H

#include <array>

#include "../core/board/types.h"
#include "score.h"

namespace q_eval {

static constexpr uint16_t MODEL_INPUT_SIZE = 512;

void InitializeModelInput(std::array<int16_t, MODEL_INPUT_SIZE>& input);
void Add(std::array<int16_t, MODEL_INPUT_SIZE>& input, q_core::cell_t cell, q_core::coord_t coord);
void SubAdd(std::array<int16_t, MODEL_INPUT_SIZE>& input, q_core::cell_t cell_first,
                      q_core::coord_t coord_first, q_core::cell_t cell_second,
                      q_core::coord_t coord_second);
void SubSubAdd(std::array<int16_t, MODEL_INPUT_SIZE>& input, q_core::cell_t cell_first,
                      q_core::coord_t coord_first, q_core::cell_t cell_second,
                      q_core::coord_t coord_second, q_core::cell_t cell_third,
                      q_core::coord_t coord_third);
score_t ApplyModel(const std::array<int16_t, MODEL_INPUT_SIZE>& input, q_core::Color move_side);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
