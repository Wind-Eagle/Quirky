#ifndef QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
#define QUIRKY_SRC_CORE_MOVES_MOVEGEN_H

#include "../board/board.h"
#include "move.h"

namespace q_core {

void GenerateAllMoves(const Board& board, MoveList& list);
void GenerateAllCaptures(const Board& board, MoveList& list);
void GenerateAllPromotions(const Board& board, MoveList& list);
void GenerateAllSimpleMoves(const Board& board, MoveList& list);

bool IsMovePseudolegal(const Board& board, Move move);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
