#include "movegen.h"

#include "../../util/bit.h"

namespace q_core {

enum class PromotionPolicy : int8_t { None = 0, OnlyPromotions = 1, All = 2 };

enum class CapturePolicy : int8_t { None = 0, OnlyCaptures = 1, All = 2 };

constexpr uint8_t PAWN_MOVE_DELTA = BOARD_SIDE;

template <uint8_t delta>
inline constexpr bitboard_t MoveAllPiecesByDelta(const bitboard_t src) {
    if constexpr (delta > 0) {
        Q_ASSERT(q_util::GetHighestBit(src) + delta < 64);
        return src << delta;
    } else {
        Q_ASSERT(q_util::GetLowestBit(src) + delta >= 0);
        return src >> (-delta);
    }
}

template <Color c>
inline constexpr uint8_t GetPawnMoveDelta() {
    return c == Color::White ? PAWN_MOVE_DELTA : -PAWN_MOVE_DELTA;
}

template <Color c>
inline constexpr uint8_t GetPawnPromotionLine() {
    return c == Color::White ? BOARD_SIDE - 2 : 1;
}

template <bool c, PromotionPolicy p, bool d>
void AddPawnMoves(const coord_t src, const coord_t dst, MoveList& list) {
    Q_STATIC_ASSERT(!(c & d));
    constexpr uint8_t MAIN_MOVE_BITS =
        (c ? CAPTURE_MOVE_BIT : 0) | (d ? PAWN_DOUBLE_MOVE_BIT : 0) | FIFTY_RULE_MOVE_BIT;
    if constexpr (p != PromotionPolicy::None) {
        list.moves[list.size++] =
            Move{.src = src, .dst = dst, .type = KNIGHT_PROMOTION_MOVE | MAIN_MOVE_BITS};
        list.moves[list.size++] =
            Move{.src = src, .dst = dst, .type = BISHOP_PROMOTION_MOVE | MAIN_MOVE_BITS};
        list.moves[list.size++] =
            Move{.src = src, .dst = dst, .type = ROOK_PROMOTION_MOVE | MAIN_MOVE_BITS};
        list.moves[list.size++] =
            Move{.src = src, .dst = dst, .type = QUEEN_PROMOTION_MOVE | MAIN_MOVE_BITS};
    } else {
        list.moves[list.size++] = Move{.src = src, .dst = dst, .type = MAIN_MOVE_BITS};
    }
}

template <Color c, PromotionPolicy p>
void GeneratePawnSimpleMoves(const Board& board, MoveList& list, const bitboard_t src,
                             const bitboard_t dst) {
    constexpr uint8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<false, p, false>(dst_coord - CURRENT_PAWN_MOVE_DELTA, dst_coord, list);
    }
}

template <Color c, PromotionPolicy p>
void GeneratePawnDoubleMoves(const Board& board, MoveList& list, const bitboard_t src,
                             const bitboard_t dst) {
    constexpr uint8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(move_dst) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<false, p, true>(dst_coord - CURRENT_PAWN_MOVE_DELTA * 2, dst_coord, list);
    }
}

template <Color c, PromotionPolicy p>
void GeneratePawnCaptures(const Board& board, MoveList& list, const bitboard_t src,
                          const bitboard_t dst) {
    constexpr uint8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst_left =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA - 1>(src & (~FILE_BITBOARD[0])) & dst;
    bitboard_t move_dst_right =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA + 1>(src & (~FILE_BITBOARD[BOARD_SIDE - 1])) &
        dst;
    while (move_dst_left) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_left);
        AddPawnMoves<true, p>(dst_coord - (CURRENT_PAWN_MOVE_DELTA - 1), dst_coord, list);
    }
    while (move_dst_right) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_right);
        AddPawnMoves<true, p, false>(dst_coord - (CURRENT_PAWN_MOVE_DELTA + 1), dst_coord, list);
    }
}

template <Color c, PromotionPolicy pp>
void GenerateAllPawnCaptures(const Board& board, MoveList& list, const bitboard_t src) {
    const bitboard_t dst = (board.en_passant_coord != UNDEFINED_COORD
                                ? board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)] |
                                      MakeBitboardFromCoord(board.en_passant_coord)
                                : board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)]);
    GeneratePawnCaptures<c, pp>(board, list, src, dst);
}

template <Color c, PromotionPolicy pp>
void GenerateAllPawnSimpleMoves(const Board& board, MoveList& list, const bitboard_t src) {
    const bitboard_t dst = board.bb_pieces[EMPTY_CELL];
    GeneratePawnSimpleMoves<c, pp>(board, list, src, dst);
    GeneratePawnDoubleMoves<c, pp>(board, list, src, dst);
}

template <Color c, CapturePolicy cp, PromotionPolicy pp>
void GenerateAllPawnMoves(const Board& board, MoveList& list) {
    const bitboard_t src =
        (pp == PromotionPolicy::All
             ? board.bb_pieces[MakeCell(c, Piece::Pawn)]
             : (pp == PromotionPolicy::None ? board.bb_pieces[MakeCell(c, Piece::Pawn)] &
                                                  (~RANK_BITBOARD[GetPawnPromotionLine<c>()])
                                            : board.bb_pieces[MakeCell(c, Piece::Pawn)] &
                                                  RANK_BITBOARD[GetPawnPromotionLine<c>()]));
    if constexpr (cp == CapturePolicy::OnlyCaptures) {
        GenerateAllPawnCaptures<c, pp>(board, list, src);
    } else if constexpr (cp == CapturePolicy::None) {
        GenerateAllPawnSimpleMoves<c, pp>(board, list, src);
    } else {
        GenerateAllPawnCaptures<c, pp>(board, list, src);
        GenerateAllPawnSimpleMoves<c, pp>(board, list, src);
    }
}

}  // namespace q_core