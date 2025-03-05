#include "attack.h"

#include "../util.h"
#include "core/board/geometry.h"
#include "core/board/types.h"
#include "core/moves/move.h"
#include "magic.h"
#include "util/bit.h"

namespace q_core {

template <Color c>
bool IsCellAttacked(const Board& board, const coord_t src) {
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
bitboard_t GetCellAttackers(const Board& board, const coord_t src) {
    Q_ASSERT(IsCoordValidAndDefined(src));
    const bitboard_t pawn_attacks = board.bb_pieces[MakeCell(c, Piece::Pawn)] &
                                    (c == Color::White ? WHITE_PAWN_REVERSED_ATTACK_BITBOARD[src]
                                                       : BLACK_PAWN_REVERSED_ATTACK_BITBOARD[src]);
    const bitboard_t knight_attacks =
        board.bb_pieces[MakeCell(c, Piece::Knight)] & KNIGHT_ATTACK_BITBOARD[src];
    const bitboard_t king_attacks =
        board.bb_pieces[MakeCell(c, Piece::King)] & KING_ATTACK_BITBOARD[src];
    const bitboard_t diagonal_attacks =
        (board.bb_pieces[MakeCell(c, Piece::Bishop)] | board.bb_pieces[MakeCell(c, Piece::Queen)]) &
        GetBishopAttackBitboard(~board.bb_pieces[EMPTY_CELL], src);
    const bitboard_t line_attacks =
        (board.bb_pieces[MakeCell(c, Piece::Rook)] | board.bb_pieces[MakeCell(c, Piece::Queen)]) &
        GetRookAttackBitboard(~board.bb_pieces[EMPTY_CELL], src);
    return pawn_attacks | knight_attacks | king_attacks | diagonal_attacks | line_attacks;
}

template <Color c>
bool IsKingInCheck(const Board& board) {
    return IsCellAttacked<GetInvertedColor(c)>(
        board, q_util::GetLowestBit(board.bb_pieces[MakeCell(c, Piece::King)]));
}

bool IsKingInCheck(const Board& board) {
    if (board.move_side == Color::White) {
        return IsKingInCheck<Color::White>(board);
    }
    return IsKingInCheck<Color::Black>(board);
}

template <Piece p>
bitboard_t GetPiecesBB(const Board& board) {
    return board.bb_pieces[MakeCell(Color::White, p)] | board.bb_pieces[MakeCell(Color::Black, p)];
}

bitboard_t GetAllAttackersBitboard(const Board& board, const coord_t src) {
    return (WHITE_PAWN_REVERSED_ATTACK_BITBOARD[src] &
            board.bb_pieces[MakeCell(Color::White, Piece::Pawn)]) |
           (BLACK_PAWN_REVERSED_ATTACK_BITBOARD[src] &
            board.bb_pieces[MakeCell(Color::Black, Piece::Pawn)]) |
           (GetPiecesBB<Piece::Knight>(board) & KNIGHT_ATTACK_BITBOARD[src]) |
           (GetPiecesBB<Piece::King>(board) & KING_ATTACK_BITBOARD[src]) |
           ((GetPiecesBB<Piece::Bishop>(board) | GetPiecesBB<Piece::Queen>(board)) &
            GetBishopAttackBitboard(~board.bb_pieces[EMPTY_CELL], src)) |
           ((GetPiecesBB<Piece::Rook>(board) | GetPiecesBB<Piece::Queen>(board)) &
            GetRookAttackBitboard(~board.bb_pieces[EMPTY_CELL], src));
}

bool IsSEENotNegative(const Board& board, const Move move, const int16_t min_score,
                      const std::array<int16_t, NUMBER_OF_CELLS>& see_cells_cost) {
    int32_t value = see_cells_cost[board.cells[move.dst]] - min_score;
    if (value < 0) {
        return false;
    }
    value = see_cells_cost[board.cells[move.src]] - value;
    if (value <= 0) {
        return true;
    }
    Color cur_color = board.move_side;
    bitboard_t occupied = (~board.bb_pieces[EMPTY_CELL]) ^ MakeBitboardFromCoord(move.src) ^
                          MakeBitboardFromCoord(move.dst);
    bitboard_t attackers = GetAllAttackersBitboard(board, move.dst);
    bitboard_t cur_color_attackers;

    const bitboard_t diag_pieces =
        GetPiecesBB<Piece::Bishop>(board) | GetPiecesBB<Piece::Queen>(board);

    const bitboard_t line_pieces =
        GetPiecesBB<Piece::Rook>(board) | GetPiecesBB<Piece::Queen>(board);

    int8_t res = 1;

    for (;;) {
        cur_color = GetInvertedColor(cur_color);
        attackers &= occupied;

        cur_color_attackers = attackers & board.bb_colors[static_cast<size_t>(cur_color)];
        if (cur_color_attackers == 0) {
            return res;
        }

        res ^= 1;

        for (const Piece piece :
             {Piece::Pawn, Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen, Piece::King}) {
            if (piece == Piece::King) {
                return (attackers & (~board.bb_colors[static_cast<size_t>(cur_color)])) ? res ^ 1
                                                                                        : res;
            }
            bitboard_t least_valuable_attacker;
            if ((least_valuable_attacker =
                     cur_color_attackers & board.bb_pieces[MakeCell(cur_color, piece)])) {
                if ((value = see_cells_cost[MakeCell(cur_color, piece)] - value) < res) {
                    return res;
                }
                q_util::ClearBit(occupied, q_util::GetLowestBit(least_valuable_attacker));
                if (piece == Piece::Pawn || piece == Piece::Bishop || piece == Piece::Queen) {
                    attackers |= GetBishopAttackBitboard(occupied, move.dst) & diag_pieces;
                }
                if (piece == Piece::Pawn || piece == Piece::Rook || piece == Piece::Queen) {
                    attackers |= GetRookAttackBitboard(occupied, move.dst) & line_pieces;
                }
                break;
            }
        }
    }
    return res;
}

template bool IsCellAttacked<Color::White>(const Board& board, coord_t src);
template bool IsCellAttacked<Color::Black>(const Board& board, coord_t src);

template bitboard_t GetCellAttackers<Color::White>(const Board& board, coord_t src);
template bitboard_t GetCellAttackers<Color::Black>(const Board& board, coord_t src);

template bool IsKingInCheck<Color::White>(const Board& board);
template bool IsKingInCheck<Color::Black>(const Board& board);

}  // namespace q_core
