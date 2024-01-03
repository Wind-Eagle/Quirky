#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_CALCER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_CALCER_H

#include "reader.h"
#include "../../../../src/eval/evaluator.h"

#include <array>

constexpr uint16_t PSQ_FEATURES_COUNT = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES;

using weight_t = float;

struct FeatureStorageWithInfo {
    std::array<int8_t, q_core::NUMBER_OF_PIECES> piece_count_storage;
    std::array<int8_t, q_eval::FEATURE_COUNT> feature_storage;
    std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES> psq_storage;
    q_eval::stage_t stage;
    uint16_t last_move_number;
    size_t game_index;
    Result result;
    weight_t weight = 1;
};

struct BoardWithFeatures {
    q_core::Board board;
    FeatureStorageWithInfo feature_storage_with_info;
};

struct BoardSetWithFeatures {
    std::vector<BoardWithFeatures> boards;
    std::string file_path;
};

BoardSetWithFeatures CalcFeatures(GameSet&& game_set_ref);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_CALCER_H
