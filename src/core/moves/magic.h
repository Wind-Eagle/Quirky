#ifndef QUIRKY_SRC_CORE_MOVES_MAGIC_H
#define QUIRKY_SRC_CORE_MOVES_MAGIC_H

#include <array>

#include "../../util/bit.h"
#include "move.h"

namespace q_core {

extern const std::array<bitboard_t, BOARD_SIZE> WHITE_PAWN_REVERSED_ATTACK_BITBOARD;
extern const std::array<bitboard_t, BOARD_SIZE> BLACK_PAWN_REVERSED_ATTACK_BITBOARD;
extern const std::array<bitboard_t, BOARD_SIZE> KNIGHT_ATTACK_BITBOARD;
extern const std::array<bitboard_t, BOARD_SIZE> KING_ATTACK_BITBOARD;
extern const std::array<bitboard_t, BOARD_SIZE> BISHOP_ATTACK_BITBOARD;
extern const std::array<bitboard_t, BOARD_SIZE> ROOK_ATTACK_BITBOARD;

struct MagicEntry {
    const bitboard_t* lookup;
    bitboard_t mask;
    bitboard_t postmask;
};

struct MagicBitboard {
  public:
    MagicBitboard();
    MagicEntry bishop_entry[64];
    MagicEntry rook_entry[64];
    bitboard_t bishop_lookup[1792];
    bitboard_t rook_lookup[65536];

  private:
    template <Piece p>
    void FillLookupTable(const std::array<uint64_t, 64>& piece_offset);
};

extern const MagicBitboard MAGIC_BITBOARD;

bitboard_t GetBishopAttackBitboard(bitboard_t occupied, coord_t src);
bitboard_t GetRookAttackBitboard(bitboard_t occupied, coord_t src);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MAGIC_H
