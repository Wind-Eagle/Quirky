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

constexpr uint8_t PAWN_MOVE_CAPTURE_BIT = (1 << 0);
constexpr uint8_t PAWN_MOVE_PROMOTION_BIT = (1 << 1);
constexpr uint8_t PAWN_MOVE_DOUBLE_BIT = (1 << 2);
constexpr uint8_t PAWN_MOVE_EN_PASSANT_BIT = (1 << 3);

template <uint8_t type>
void AddPawnMoves(const coord_t src, const coord_t dst, Move* list, size_t& size) {
    constexpr bool C = (type & PAWN_MOVE_CAPTURE_BIT);
    constexpr bool P = (type & PAWN_MOVE_PROMOTION_BIT);
    constexpr bool D = (type & PAWN_MOVE_DOUBLE_BIT);
    constexpr bool E = (type & PAWN_MOVE_EN_PASSANT_BIT);
    Q_STATIC_ASSERT(!(C & D) && !(E && !C) && !(D && E) && !(D && P) && !(E && P));
    constexpr uint8_t MAIN_MOVE_BITS = (C ? CAPTURE_MOVE_BIT : 0) | (D ? PAWN_DOUBLE_MOVE_BIT : 0) |
                                       FIFTY_RULE_MOVE_BIT | (E ? EN_PASSANT_MOVE_BIT : 0);
    if constexpr (P) {
        list[size++] = Move{.src = src, .dst = dst, .type = KNIGHT_PROMOTION_MOVE_TYPE | MAIN_MOVE_BITS};
        list[size++] = Move{.src = src, .dst = dst, .type = BISHOP_PROMOTION_MOVE_TYPE | MAIN_MOVE_BITS};
        list[size++] = Move{.src = src, .dst = dst, .type = ROOK_PROMOTION_MOVE_TYPE | MAIN_MOVE_BITS};
        list[size++] = Move{.src = src, .dst = dst, .type = QUEEN_PROMOTION_MOVE_TYPE | MAIN_MOVE_BITS};
    } else {
        list[size++] = Move{.src = src, .dst = dst, .type = MAIN_MOVE_BITS};
    }
}

template <Color c, bool p>
void GeneratePawnSimpleMoves(const Board& board, Move* list, const bitboard_t src,
                             const bitboard_t dst, size_t& size) {
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<p ? PAWN_MOVE_PROMOTION_BIT : 0>(dst_coord - CURRENT_PAWN_MOVE_DELTA,
                                                      dst_coord, list, size);
    }
}

template <Color c>
void GeneratePawnDoubleMoves(const Board& board, Move* list, const bitboard_t src,
                             const bitboard_t dst, size_t& size) {
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(move_dst) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<PAWN_MOVE_DOUBLE_BIT>(dst_coord - CURRENT_PAWN_MOVE_DELTA * 2, dst_coord, list,
                                           size);
    }
}

template <Color c, bool p>
void GeneratePawnCaptures(const Board& board, Move* list, const bitboard_t src,
                          const bitboard_t dst, size_t& size) {
    if constexpr (p) {
        if (Q_LIKELY(src == 0)) {
            return;
        }
    }
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta<c>();
    const bitboard_t move_left =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA - 1>(src & (~FILE_BITBOARD[0]));
    bitboard_t move_dst_left = move_left & dst;
    while (move_dst_left) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_left);
        AddPawnMoves<(p ? PAWN_MOVE_PROMOTION_BIT : 0) | PAWN_MOVE_CAPTURE_BIT>(
            dst_coord - (CURRENT_PAWN_MOVE_DELTA - 1), dst_coord, list, size);
    }
    const bitboard_t move_right =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA + 1>(src & (~FILE_BITBOARD[BOARD_SIDE - 1]));
    bitboard_t move_dst_right = move_right & dst;
    while (move_dst_right) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_right);
        AddPawnMoves<(p ? PAWN_MOVE_PROMOTION_BIT : 0) | PAWN_MOVE_CAPTURE_BIT>(
            dst_coord - (CURRENT_PAWN_MOVE_DELTA + 1), dst_coord, list, size);
    }
    if constexpr (!p) {
        Q_ASSERT(IsCoordValidAndDefined(board.en_passant_coord));
        if (Q_UNLIKELY(board.en_passant_coord != NO_ENPASSANT_COORD)) {
            move_dst_left = move_left & MakeBitboardFromCoord(board.en_passant_coord);
            if (Q_UNLIKELY(move_dst_left)) {
                const coord_t dst_coord = board.en_passant_coord;
                AddPawnMoves<PAWN_MOVE_CAPTURE_BIT | PAWN_MOVE_EN_PASSANT_BIT>(
                    dst_coord - (CURRENT_PAWN_MOVE_DELTA - 1), dst_coord, list, size);
            }
            move_dst_right = move_right & MakeBitboardFromCoord(board.en_passant_coord);
            if (Q_UNLIKELY(move_dst_right)) {
                const coord_t dst_coord = board.en_passant_coord;
                AddPawnMoves<PAWN_MOVE_CAPTURE_BIT | PAWN_MOVE_EN_PASSANT_BIT>(
                    dst_coord - (CURRENT_PAWN_MOVE_DELTA + 1), dst_coord, list, size);
            }
        }
    }
}

template <Color c, bool p>
void GenerateAllPawnCaptures(const Board& board, Move* list, const bitboard_t src, size_t& size) {
    GeneratePawnCaptures<c, p>(board, list, src,
                               board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))], size);
}

template <Color c, bool p>
void GenerateAllPawnSimpleMoves(const Board& board, Move* list, const bitboard_t src,
                                size_t& size) {
    const bitboard_t dst = board.bb_pieces[EMPTY_CELL];
    GeneratePawnSimpleMoves<c, p>(board, list, src, dst, size);
    if constexpr (!p) {
        GeneratePawnDoubleMoves<c>(board, list, src & GetPawnDoubleMoveRank<c>(), dst, size);
    }
}

template <Color c, CapturePolicy cp, PromotionPolicy pp>
void GenerateAllPawnMoves(const Board& board, Move* list, size_t& size) {
    if constexpr (pp == PromotionPolicy::All) {
        GenerateAllPawnMoves<c, cp, PromotionPolicy::None>(board, list, size);
        GenerateAllPawnMoves<c, cp, PromotionPolicy::OnlyPromotions>(board, list, size);
    } else {
        const bitboard_t src =
            board.bb_pieces[MakeCell(c, Piece::Pawn)] &
            (pp == PromotionPolicy::None ? (~RANK_BITBOARD[GetPawnPromotionLine<c>()])
                                         : RANK_BITBOARD[GetPawnPromotionLine<c>()]);
        if constexpr (cp != CapturePolicy::OnlyCaptures) {
            GenerateAllPawnSimpleMoves<c, pp != PromotionPolicy::None>(board, list, src, size);
        }
        if constexpr (cp != CapturePolicy::None) {
            GenerateAllPawnCaptures<c, pp != PromotionPolicy::None>(board, list, src, size);
        }
    }
}

constexpr Move WHITE_KINGSIDE_CASTLING_MOVE =
    Move{WHITE_KING_INITIAL_POSITION, WHITE_KING_INITIAL_POSITION + 2, KINGSIDE_CASTLING_MOVE_TYPE};
constexpr Move WHITE_QUEENSIDE_CASTLING_MOVE =
    Move{WHITE_KING_INITIAL_POSITION, WHITE_KING_INITIAL_POSITION - 2, QUEENSIDE_CASTLING_MOVE_TYPE};
constexpr Move BLACK_KINGSIDE_CASTLING_MOVE =
    Move{BLACK_KING_INITIAL_POSITION, BLACK_KING_INITIAL_POSITION + 2, KINGSIDE_CASTLING_MOVE_TYPE};
constexpr Move BLACK_QUEENSIDE_CASTLING_MOVE =
    Move{BLACK_KING_INITIAL_POSITION, BLACK_KING_INITIAL_POSITION - 2, QUEENSIDE_CASTLING_MOVE_TYPE};
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
void GenerateCastling(const Board& board, Move* list, size_t& size) {
    if constexpr (c == Color::White) {
        if (Q_UNLIKELY(IsCastlingAllowed(board.castling, Castling::WhiteAll))) {
            if (!((~board.bb_pieces[EMPTY_CELL]) & WHITE_KINGSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::WhiteKingside)) {
                list[size++] = WHITE_KINGSIDE_CASTLING_MOVE;
            }
            if (!((~board.bb_pieces[EMPTY_CELL]) & WHITE_QUEENSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::WhiteQueenside)) {
                list[size++] = WHITE_QUEENSIDE_CASTLING_MOVE;
            }
        }
    } else {
        if (Q_UNLIKELY(IsCastlingAllowed(board.castling, Castling::BlackAll))) {
            if (!((~board.bb_pieces[EMPTY_CELL]) & BLACK_KINGSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::BlackKingside)) {
                list[size++] = BLACK_KINGSIDE_CASTLING_MOVE;
            }
            if (!((~board.bb_pieces[EMPTY_CELL]) & BLACK_QUEENSIDE_CASTLING_MOVE_BITBOARD) &&
                IsCastlingAllowed(board.castling, Castling::BlackQueenside)) {
                list[size++] = BLACK_QUEENSIDE_CASTLING_MOVE;
            }
        }
    }
    Q_ASSERT(size < MAX_MOVES_COUNT);
}

template <Color c, Piece p, CapturePolicy cp>
void GenerateAllKNBRMoves(const Board& board, Move* list, const bitboard_t src, size_t& size) {
    Q_STATIC_ASSERT(p == Piece::Knight || p == Piece::Bishop || p == Piece::Rook ||
                    p == Piece::King);
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
            while (move_dst) {
                const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
                list[size++] = Move{src_coord, dst_coord, CAPTURE_MOVE_BIT | FIFTY_RULE_MOVE_BIT};
            }
        }
        if constexpr (cp != CapturePolicy::OnlyCaptures) {
            bitboard_t move_dst = move_dst_init & board.bb_pieces[EMPTY_CELL];
            while (move_dst) {
                const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
                list[size++] = Move{src_coord, dst_coord, 0};
            }
        }
    }
    if constexpr (p == Piece::King && cp != CapturePolicy::OnlyCaptures) {
        GenerateCastling<c>(board, list, size);
    }
}

template <Color c, Piece p, CapturePolicy cp>
void GenerateAllKNBRQMoves(const Board& board, Move* list, size_t& size) {
    Q_STATIC_ASSERT(p == Piece::Knight || p == Piece::Bishop || p == Piece::Rook ||
                    p == Piece::King);
    if constexpr (p == Piece::Bishop) {
        GenerateAllKNBRMoves<c, Piece::Bishop, cp>(board, list,
                                                   board.bb_pieces[MakeCell(c, Piece::Bishop)] |
                                                       board.bb_pieces[MakeCell(c, Piece::Queen)],
                                                   size);
    } else if constexpr (p == Piece::Rook) {
        GenerateAllKNBRMoves<c, Piece::Rook, cp>(
            board, list,
            board.bb_pieces[MakeCell(c, Piece::Rook)] | board.bb_pieces[MakeCell(c, Piece::Queen)],
            size);
    } else {
        GenerateAllKNBRMoves<c, p, cp>(board, list, board.bb_pieces[MakeCell(c, p)], size);
    }
}

void GenerateAllMoves(const Board& board, MoveList& list) {
    Q_ASSERT(board.IsValid());
    size_t size = 0;
    if (board.move_side == q_core::Color::White) {
        q_core::GenerateAllPawnMoves<q_core::Color::White, q_core::CapturePolicy::All,
                                     q_core::PromotionPolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Knight,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Bishop,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Rook,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::King,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
    } else {
        q_core::GenerateAllPawnMoves<q_core::Color::Black, q_core::CapturePolicy::All,
                                     q_core::PromotionPolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Knight,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Bishop,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Rook,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
        q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::King,
                                      q_core::CapturePolicy::All>(board, list.moves, size);
    }
    list.size = size;
}

}  // namespace q_core