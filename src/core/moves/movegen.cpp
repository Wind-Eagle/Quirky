#include "movegen.h"

#include "../../util/bit.h"
#include "../board/geometry.h"
#include "../util.h"
#include "magic.h"

namespace q_core {

enum class PromotionPolicy : int8_t { None = 0, OnlyPromotions = 1, All = 2 };

enum class CapturePolicy : int8_t { None = 0, OnlyCaptures = 1, All = 2 };

constexpr uint8_t PAWN_MOVE_DELTA = BOARD_SIDE;

template <Color c>
inline constexpr int8_t GetPawnMoveDelta() {
    return c == Color::White ? PAWN_MOVE_DELTA : -PAWN_MOVE_DELTA;
}

template <Color c>
inline constexpr uint8_t GetPawnPromotionLine() {
    return c == Color::White ? BOARD_SIDE - 2 : 1;
}

template <Color c>
inline constexpr bitboard_t GetPawnDoubleMoveRank() {
    return c == Color::White ? RANK_BITBOARD[1] : RANK_BITBOARD[BOARD_SIDE - 2];
}

template <int8_t delta>
inline constexpr bitboard_t MoveAllPiecesByDelta(const bitboard_t b) {
    return static_cast<bitboard_t>(q_util::MoveAllBitsByDelta<delta>(b));
}

template <bool c, bool p, bool d>
void AddPawnMoves(const coord_t src, const coord_t dst, MoveList& list) {
    Q_STATIC_ASSERT(!(c & d));
    Q_ASSERT(IsCoordValid(src) && IsCoordValid(dst) && list.size < MAX_MOVES_COUNT);
    constexpr uint8_t MAIN_MOVE_BITS =
        (c ? CAPTURE_MOVE_BIT : 0) | (d ? PAWN_DOUBLE_MOVE_BIT : 0) | FIFTY_RULE_MOVE_BIT;
    if constexpr (p) {
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

template <Color c, bool p>
void GeneratePawnSimpleMoves(const Board& board, MoveList& list, const bitboard_t src,
                             const bitboard_t dst) {
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<false, p, false>(dst_coord - CURRENT_PAWN_MOVE_DELTA, dst_coord, list);
    }
}

template <Color c>
void GeneratePawnDoubleMoves(const Board& board, MoveList& list, const bitboard_t src,
                             const bitboard_t dst) {
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(move_dst) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<false, false, true>(dst_coord - CURRENT_PAWN_MOVE_DELTA * 2, dst_coord, list);
    }
}

template <Color c, bool p>
void GeneratePawnCaptures(const Board& board, MoveList& list, const bitboard_t src,
                          const bitboard_t dst) {
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst_left =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA - 1>(src & (~FILE_BITBOARD[0])) & dst;
    bitboard_t move_dst_right =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA + 1>(src & (~FILE_BITBOARD[BOARD_SIDE - 1])) &
        dst;
    while (move_dst_left) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_left);
        AddPawnMoves<true, p, false>(dst_coord - (CURRENT_PAWN_MOVE_DELTA - 1), dst_coord, list);
    }
    while (move_dst_right) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_right);
        AddPawnMoves<true, p, false>(dst_coord - (CURRENT_PAWN_MOVE_DELTA + 1), dst_coord, list);
    }
}

uint64_t GetPawnCaptureDstBitboard(const uint64_t dst, const coord_t en_passant_coord) {
    uint64_t ans = dst;
    if (Q_UNLIKELY(en_passant_coord != UNDEFINED_COORD)) {
        q_util::SetBit(ans, en_passant_coord);
    }
    return ans;
}

template <Color c, bool p>
void GenerateAllPawnCaptures(const Board& board, MoveList& list, const bitboard_t src) {
    Q_ASSERT(IsCoordValid(src) && list.size < MAX_MOVES_COUNT);
    const bitboard_t dst = GetPawnCaptureDstBitboard(
        board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))], board.en_passant_coord);
    GeneratePawnCaptures<c, p>(board, list, src, dst);
}

template <Color c, bool p>
void GenerateAllPawnSimpleMoves(const Board& board, MoveList& list, const bitboard_t src) {
    Q_ASSERT(IsCoordValid(src) && list.size < MAX_MOVES_COUNT);
    const bitboard_t dst = board.bb_pieces[EMPTY_CELL];
    GeneratePawnSimpleMoves<c, p>(board, list, src, dst);
    if constexpr (!p) {
        GeneratePawnDoubleMoves<c>(board, list, src & GetPawnDoubleMoveRank<c>(), dst);
    }
}

template <Color c, CapturePolicy cp, PromotionPolicy pp>
void GenerateAllPawnMoves(const Board& board, MoveList& list) {
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    if constexpr (pp == PromotionPolicy::All) {
        GenerateAllPawnMoves<c, cp, PromotionPolicy::None>(board, list);
        GenerateAllPawnMoves<c, cp, PromotionPolicy::OnlyPromotions>(board, list);
    } else {
        const bitboard_t src =
            board.bb_pieces[MakeCell(c, Piece::Pawn)] &
            (pp == PromotionPolicy::None ? (~RANK_BITBOARD[GetPawnPromotionLine<c>()])
                                         : RANK_BITBOARD[GetPawnPromotionLine<c>()]);
        GenerateAllPawnSimpleMoves<c, pp != PromotionPolicy::None>(board, list, src);
        GenerateAllPawnCaptures<c, pp != PromotionPolicy::None>(board, list, src);
    }
}

constexpr Move WHITE_KINGSIDE_CASTLING_MOVE =
    Move{WHITE_KING_INITIAL_POSITION, WHITE_KING_INITIAL_POSITION + 2, KINGSIDE_CASTLING_MOVE};
constexpr Move WHITE_QUEENSIDE_CASTLING_MOVE =
    Move{WHITE_KING_INITIAL_POSITION, WHITE_KING_INITIAL_POSITION - 2, KINGSIDE_CASTLING_MOVE};
constexpr Move BLACK_KINGSIDE_CASTLING_MOVE =
    Move{BLACK_KING_INITIAL_POSITION, BLACK_KING_INITIAL_POSITION + 2, QUEENSIDE_CASTLING_MOVE};
constexpr Move BLACK_QUEENSIDE_CASTLING_MOVE =
    Move{BLACK_KING_INITIAL_POSITION, BLACK_KING_INITIAL_POSITION - 2, QUEENSIDE_CASTLING_MOVE};
constexpr bitboard_t WHITE_KINGSIDE_CASTLING_MOVE_BITBOARD =
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION + 1) |
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION + 2);
constexpr bitboard_t WHITE_QUEENSIDE_CASTLING_MOVE_BITBOARD =
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION - 1) |
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION - 2) |
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION - 3);
constexpr bitboard_t BLACK_KINGSIDE_CASTLING_MOVE_BITBOARD =
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION + 1) |
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION + 2);
constexpr bitboard_t BLACK_QUEENSIDE_CASTLING_MOVE_BITBOARD =
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION - 1) |
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION - 2) |
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION - 3);

template <Color c>
void GenerateCastling(const Board& board, MoveList& list) {
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    if constexpr (c == Color::White) {
        if (Q_UNLIKELY(IsCastlingAllowed(board.castling, Castling::WhiteAll))) {
            if (!((~board.bb_pieces[EMPTY_CELL]) & WHITE_KINGSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::WhiteKingside)) {
                list.moves[list.size++] = WHITE_KINGSIDE_CASTLING_MOVE;
            }
            if (!((~board.bb_pieces[EMPTY_CELL]) & WHITE_QUEENSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::WhiteQueenside)) {
                list.moves[list.size++] = WHITE_QUEENSIDE_CASTLING_MOVE;
            }
        }
    } else {
        if (Q_UNLIKELY(IsCastlingAllowed(board.castling, Castling::BlackAll))) {
            if (!((~board.bb_pieces[EMPTY_CELL]) & BLACK_KINGSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::BlackKingside)) {
                list.moves[list.size++] = BLACK_KINGSIDE_CASTLING_MOVE;
            }
            if (!((~board.bb_pieces[EMPTY_CELL]) & BLACK_QUEENSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::BlackQueenside)) {
                list.moves[list.size++] = BLACK_QUEENSIDE_CASTLING_MOVE;
            }
        }
    }
}

template <Color c, Piece p, CapturePolicy cp>
void GenerateAllKNBRMoves(const Board& board, MoveList& list, const bitboard_t src) {
    Q_STATIC_ASSERT(p == Piece::Knight || p == Piece::Bishop || p == Piece::Rook ||
                    p == Piece::King);
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    bitboard_t pieces_src = src;
    while (pieces_src) {
        const coord_t src_coord = q_util::ExtractLowestBit(pieces_src);
        bitboard_t move_dst_init;
        if constexpr (p == Piece::Knight) {
            move_dst_init = KNIGHT_ATTACK_BITBOARD[src_coord];
        } else if constexpr (p == Piece::King) {
            move_dst_init = KING_ATTACK_BITBOARD[src_coord];
        } else if constexpr (p == Piece::Bishop) {
            move_dst_init = GetBishopAttackBitboard(~board.bb_pieces[EMPTY_CELL], src_coord);
        } else {
            move_dst_init = GetRookAttackBitboard(~board.bb_pieces[EMPTY_CELL], src_coord);
        }
        if constexpr (cp != CapturePolicy::None) {
            bitboard_t move_dst =
                move_dst_init & board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))];
            while (Q_LIKELY(move_dst)) {
                const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
                list.moves[list.size++] = Move{.src = src_coord,
                                               .dst = dst_coord,
                                               .type = CAPTURE_MOVE_BIT | FIFTY_RULE_MOVE_BIT};
            }
        }
        if constexpr (cp != CapturePolicy::OnlyCaptures) {
            bitboard_t move_dst = move_dst_init & board.bb_pieces[EMPTY_CELL];
            while (Q_LIKELY(move_dst)) {
                const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
                list.moves[list.size++] = Move{.src = src_coord, .dst = dst_coord, .type = 0};
            }
        }
    }
    if constexpr (p == Piece::King && cp != CapturePolicy::OnlyCaptures) {
        GenerateCastling<c>(board, list);
    }
}

template <Color c, Piece p, CapturePolicy cp>
void GenerateAllKNBRQMoves(const Board& board, MoveList& list) {
    Q_STATIC_ASSERT(p == Piece::Knight || p == Piece::Bishop || p == Piece::Rook ||
                    p == Piece::Queen || p == Piece::King);
    Q_ASSERT(list.size < MAX_MOVES_COUNT);
    if constexpr (p == Piece::Queen) {
        GenerateAllKNBRMoves<c, Piece::Bishop, cp>(board, list, board.bb_pieces[MakeCell(c, p)]);
        GenerateAllKNBRMoves<c, Piece::Rook, cp>(board, list, board.bb_pieces[MakeCell(c, p)]);
    } else {
        GenerateAllKNBRMoves<c, p, cp>(board, list, board.bb_pieces[MakeCell(c, p)]);
    }
}

void GenerateAllMoves(const Board& board, MoveList& list) {
    if (board.move_side == q_core::Color::White) {
        q_core::GenerateAllPawnMoves<q_core::Color::White, q_core::CapturePolicy::All,
                                     q_core::PromotionPolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Knight,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Bishop,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Rook,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Queen,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::King,
                                      q_core::CapturePolicy::All>(board, list);
    } else {
        q_core::GenerateAllPawnMoves<q_core::Color::Black, q_core::CapturePolicy::All,
                                     q_core::PromotionPolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Knight,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Bishop,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Rook,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Queen,
                                      q_core::CapturePolicy::All>(board, list);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::King,
                                      q_core::CapturePolicy::All>(board, list);
    }
}

}  // namespace q_core