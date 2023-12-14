#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_CALCER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_CALCER_H

#include "reader.h"
#include "../../../../src/eval/evaluator.h"

#include <array>

constexpr uint16_t PSQ_FEATURES_COUNT = q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES;

struct FeatureStorageWithInfo {
    std::array<int8_t, q_eval::FEATURE_COUNT> feature_storage;
    std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES> psq_storage;
    q_eval::stage_t stage;
    size_t game_index;
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
