#ifndef QUIRKY_SRC_CORE_MOVES_BOARD_MANIPULATION_H
#define QUIRKY_SRC_CORE_MOVES_BOARD_MANIPULATION_H

#include "../board/board.h"
#include "move.h"

namespace q_core {

struct MakeMoveInfo {
    hash_t hash;
    coord_t en_passant;
    Castling castling;
    uint8_t fifty_rule_move_counter;
    cell_t dst_cell;
};

void MakeMove(Board& board, Move move, MakeMoveInfo& info);
void UnmakeMove(Board& board, Move move, const MakeMoveInfo& info);
bool WasMoveLegal(const Board& board, Move move);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_BOARD_MANIPULATION_H
