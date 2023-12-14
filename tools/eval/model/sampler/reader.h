#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_READER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_READER_H

#include "../../../../src/core/board/board.h"

#include <vector>

enum class Result: int8_t {
    WhiteWins = -1,
    Draw = 0,
    BlackWins = 1
};

struct GameHeader {
    Result result;
    std::string name;
};

struct GameStartposWithResult {
    GameHeader header;
    q_core::Board board;
};

struct GameWithResult {
    GameHeader header;
    std::vector<q_core::Board> boards;
};

struct GameSet {
    std::vector<GameWithResult> games;
    std::string file_path;
};

GameSet ReadGames(std::string_view path);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_READER_H
