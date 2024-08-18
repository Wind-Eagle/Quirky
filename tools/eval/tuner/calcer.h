#ifndef QUIRKY_TOOLS_EVAL_TUNER_PARSER_CALCER_H
#define QUIRKY_TOOLS_EVAL_TUNER_PARSER_CALCER_H

#include "reader.h"

#include "../../../src/eval/model.h"
#include "../../../src/eval/psq.h"

struct CalcerResult {
    double loss;
    size_t number_of_positions;
};

constexpr uint16_t COMPRESSED_PSQ_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES;
constexpr uint16_t PSQ_AND_FEATURE_COUNT = (COMPRESSED_PSQ_SIZE + q_eval::FEATURE_COUNT + q_core::NUMBER_OF_PIECES) * 2;
constexpr uint16_t TAPERED_EVAL_BOUND = COMPRESSED_PSQ_SIZE + q_eval::FEATURE_COUNT + q_core::NUMBER_OF_PIECES;

CalcerResult GetCalcerResult(Game& game);

#endif  // QUIRKY_TOOLS_EVAL_TUNER_PARSER_CALCER_H