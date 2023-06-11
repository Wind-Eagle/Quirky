#ifndef QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
#define QUIRKY_SRC_CORE_MOVES_MOVEGEN_H

#include "move.h"

namespace q_core {

struct PseudolegalMovegen {
    void GenerateAllMoves();
    void GenerateCaptures();
    void GeneratePromotions();
    void GenerateSimpleMoves();
    MoveList moves;
};

struct LegalMovegen {
    void Init();
    void GenerateAllMoves();
    void GenerateCaptures();
    void GeneratePromotions();
    void GenerateSimpleMoves();
    MoveList moves;
    bitboard_t attackers_bitboard;
};

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVEGEN_H
