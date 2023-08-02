#include "attack.h"

#include "../util.h"
#include "magic.h"

namespace q_core {

template <Color c>
bool IsCellAttacked(const Board& board, coord_t src) {
    Q_ASSERT(IsCoordValidAndDefined(src));
    if (board.bb_pieces[MakeCell(c, Piece::Pawn)] &
        (c == Color::White ? WHITE_PAWN_REVERSED_ATTACK_BITBOARD[src]
                           : BLACK_PAWN_REVERSED_ATTACK_BITBOARD[src])) {
        return true;
    }
    if (board.bb_pieces[MakeCell(c, Piece::Knight)] & KNIGHT_ATTACK_BITBOARD[src]) {
        return true;
    }
    if (board.bb_pieces[MakeCell(c, Piece::King)] & KING_ATTACK_BITBOARD[src]) {
        return true;
    }
    if ((board.bb_pieces[MakeCell(c, Piece::Bishop)] | board.bb_pieces[MakeCell(c, Piece::Queen)]) &
        GetBishopAttackBitboard(~board.bb_pieces[EMPTY_CELL], src)) {
        return true;
    }
    if ((board.bb_pieces[MakeCell(c, Piece::Rook)] | board.bb_pieces[MakeCell(c, Piece::Queen)]) &
        GetRookAttackBitboard(~board.bb_pieces[EMPTY_CELL], src)) {
        return true;
    }
    return false;
}

template <Color c>
bool IsKingInCheck(const Board& board) {
    return IsCellAttacked<GetInvertedColor(c)>(board, q_util::GetLowestBit(board.bb_pieces[MakeCell(c, Piece::King)]));
}

template bool IsCellAttacked<Color::White>(const Board& board, coord_t src);
template bool IsCellAttacked<Color::Black>(const Board& board, coord_t src);

template bool IsKingInCheck<Color::White>(const Board& board);
template bool IsKingInCheck<Color::Black>(const Board& board);

}  // namespace q_core