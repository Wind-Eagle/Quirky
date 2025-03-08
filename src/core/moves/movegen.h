#ifndef QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
#define QUIRKY_SRC_CORE_MOVES_MOVEGEN_H

#include "../board/board.h"
#include "core/board/types.h"
#include "move.h"

namespace q_core {

class Movegen {
  public:
    explicit Movegen(const Board& board);
    void GenerateAllMoves(const Board& board, MoveList& list);
    void GenerateAllCaptures(const Board& board, MoveList& list);
    void GenerateAllPromotions(const Board& board, MoveList& list);
    void GenerateAllSimpleMoves(const Board& board, MoveList& list);

  private:
    enum class CheckKind : int8_t { None = 0, Single = 1, Double = 2 };
    bitboard_t dst_mask_ = FULL_BITBOARD;
    CheckKind check_kind_;
};

bool IsMovePseudolegal(const Board& board, Move move);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
