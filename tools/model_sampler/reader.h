#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_READER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_READER_H

#include <fstream>
#include <vector>

#include "../../src/core/board/board.h"

enum class Result : int8_t { CurSideWins = -1, Draw = 0, OtherSideWins = 1 };

struct Position {
    q_core::Board board;
    float target;
};

struct PositionSet {
    std::vector<Position> positions;
};

PositionSet ReadPositions(std::ifstream& in, size_t batch_size);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_READER_H
