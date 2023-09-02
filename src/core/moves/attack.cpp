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
    return IsCellAttacked<GetInvertedColor(c)>(
        board, q_util::GetLowestBit(board.bb_pieces[MakeCell(c, Piece::King)]));
}

template <Color c>
bitboard_t GetPawnAttacks(const bitboard_t pawns) {
    constexpr int8_t PAWN_MOVE_DELTA = BOARD_SIDE;
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA =
        (c == Color::White ? PAWN_MOVE_DELTA : -PAWN_MOVE_DELTA);
    return static_cast<bitboard_t>(
        q_util::MoveAllBitsByDelta<CURRENT_PAWN_MOVE_DELTA - 1>(pawns & (~FILE_BITBOARD[0])) |
        q_util::MoveAllBitsByDelta<CURRENT_PAWN_MOVE_DELTA + 1>(pawns &
                                                                (~FILE_BITBOARD[BOARD_SIDE - 1])));
}

template bool IsCellAttacked<Color::White>(const Board& board, coord_t src);
template bool IsCellAttacked<Color::Black>(const Board& board, coord_t src);

template bool IsKingInCheck<Color::White>(const Board& board);
template bool IsKingInCheck<Color::Black>(const Board& board);

template bitboard_t GetPawnAttacks<Color::White>(const bitboard_t pawns);
template bitboard_t GetPawnAttacks<Color::Black>(const bitboard_t pawns);

}  // namespace q_core