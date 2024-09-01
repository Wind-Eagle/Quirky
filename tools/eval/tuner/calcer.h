#ifndef QUIRKY_TOOLS_EVAL_TUNER_PARSER_CALCER_H
#define QUIRKY_TOOLS_EVAL_TUNER_PARSER_CALCER_H

#include <memory>

#include "../../../src/eval/model.h"
#include "../../../src/eval/psq.h"
#include "dataset.h"
#include "reader.h"

enum class CalcerFeatureChangeType { PieceWeight = 0, PSQ = 1, Model = 2 };

struct CalcerParams {
    q_core::Piece changed_piece;
    q_core::coord_t changed_coord;
    CalcerFeatureChangeType change_type;
    bool revert_last_change;
};

constexpr uint16_t COMPRESSED_PSQ_SIZE = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES;
constexpr uint16_t PSQ_AND_FEATURE_COUNT =
    (COMPRESSED_PSQ_SIZE + q_eval::FEATURE_COUNT + q_core::NUMBER_OF_PIECES) * 2;
constexpr uint16_t TAPERED_EVAL_BOUND =
    COMPRESSED_PSQ_SIZE + q_eval::FEATURE_COUNT + q_core::NUMBER_OF_PIECES;

double GetCalcerResult(std::shared_ptr<Element> element, CalcerParams params);

#endif  // QUIRKY_TOOLS_EVAL_TUNER_PARSER_CALCER_H