#ifndef QUIRKY_SRC_CORE_MOVES_BOARD_MANIPULATION_H
#define QUIRKY_SRC_CORE_MOVES_BOARD_MANIPULATION_H

#include "../board/board.h"
#include "move.h"

namespace q_core {

struct MadeMoveInfo {
    cell_t dst_cell;
    hash_t hash;
    Castling castling;
    coord_t en_passant;
    uint8_t fifty_rule_move_counter;
};

void MakeMove(Board& board, Move move, MadeMoveInfo& info);
bool MakeMoveWithCheck(Board& board, Move move, MadeMoveInfo& info);
void UnmakeMove(Board& board, Move move, const MadeMoveInfo& info);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_BOARD_MANIPULATION_H
