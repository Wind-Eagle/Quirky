#ifndef QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
#define QUIRKY_SRC_CORE_MOVES_MOVEGEN_H

#include "../board/board.h"
#include "move.h"

namespace q_core {

struct SimpleMovegen {
    static void GenerateAllMoves(const Board& board, MoveList& list);
};

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
