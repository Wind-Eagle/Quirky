#ifndef QUIRKY_TOOLS_EVAL_TUNER_PARSER_READER_H
#define QUIRKY_TOOLS_EVAL_TUNER_PARSER_READER_H

#include <fstream>
#include <queue>

#include "../../../src/core/board/board.h"
#include "../../../src/eval/model.h"

enum class Result : int8_t { WhiteWins = -1, Draw = 0, BlackWins = 1 };

struct GameHeader {
    uint16_t last_move_number;
    size_t game_index;
    Result result;
};

struct Game {
    std::string start_board_fen;
    std::vector<std::string> moves;
    GameHeader header;
};

class Reader {
  public:
    explicit Reader(const std::string& filename);
    Game GetNextGame();
    bool HasNext() const;

  private:
    Game ReadGame();
    std::ifstream in_;
    size_t index_ = 0;
};

#endif  // QUIRKY_TOOLS_EVAL_TUNER_PARSER_READER_H
