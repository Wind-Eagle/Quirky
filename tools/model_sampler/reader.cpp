#include "reader.h"

#include <fstream>

#include "../../src/core/moves/board_manipulation.h"
#include "../../src/core/moves/move.h"
#include "../../src/util/string.h"
#include "core/board/types.h"
#include "util/io.h"

Position ReadPosition(std::ifstream& in) {
    const auto res = q_util::ReadLine(in);
    const auto parts = q_util::SplitString(*res, ',');
    const auto& fen = parts[0];
    float target = std::stod(parts.back()) * 2 -1;
    q_core::Board board;
    board.MakeFromFEN(fen);
    if (board.move_side == q_core::Color::Black) {
        target *= -1;
    }
    return Position{board, target};
}

PositionSet ReadPositions(std::ifstream& in, size_t batch_size) {
    PositionSet position_set;
    std::string tmp;
    while (position_set.positions.size() < batch_size) {
        if (in.peek() == '\n') {
            std::getline(in, tmp);
            continue;
        }
        if (in.eof()) {
            break;
        }
        position_set.positions.push_back(ReadPosition(in));
    }
    return position_set;
}
