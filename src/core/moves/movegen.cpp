#include "movegen.h"

#include "../../util/bit.h"
#include "../board/geometry.h"
#include "../util.h"
#include "magic.h"
#include "move.h"

namespace q_core {

enum class PromotionPolicy : int8_t { None = 0, OnlyPromotions = 1, All = 2 };

enum class CapturePolicy : int8_t { None = 0, OnlyCaptures = 1, All = 2 };

template <int8_t delta>
inline constexpr bitboard_t MoveAllPiecesByDelta(const bitboard_t b) {
    return static_cast<bitboard_t>(q_util::MoveAllBitsByDelta<delta>(b));
}

template <MoveBasicType basic_type, bool is_capture, bool is_promotion>
void AddPawnMoves(const coord_t src, const coord_t dst, Move* list, size_t& size) {
    if constexpr (is_promotion) {
        list[size++] = ConstructMove(src, dst, GetMoveType<MoveBasicType::KnightPromotion>(is_capture));
        list[size++] = ConstructMove(src, dst, GetMoveType<MoveBasicType::BishopPromotion>(is_capture));
        list[size++] = ConstructMove(src, dst, GetMoveType<MoveBasicType::RookPromotion>(is_capture));
        list[size++] = ConstructMove(src, dst, GetMoveType<MoveBasicType::QueenPromotion>(is_capture));
    } else {
        list[size++] = ConstructMove(src, dst, GetMoveType<basic_type>(is_capture));
    }
}

template <Color c, bool p>
void GeneratePawnSimpleMoves(Move* list, const bitboard_t src, const bitboard_t dst, size_t& size) {
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta(c);
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<MoveBasicType::Simple, false, p>(
            dst_coord - CURRENT_PAWN_MOVE_DELTA, dst_coord, list, size);
    }
}

template <Color c>
void GeneratePawnDoubleMoves(Move* list, const bitboard_t src, const bitboard_t dst, size_t& size) {
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta(c);
    bitboard_t move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(src) & dst;
    move_dst = MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA>(move_dst) & dst;
    while (move_dst) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
        AddPawnMoves<MoveBasicType::PawnDouble, false, false>(
            dst_coord - CURRENT_PAWN_MOVE_DELTA * 2, dst_coord, list, size);
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
    constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta(c);
    const bitboard_t move_left =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA - 1>(src & (~FILE_BITBOARD[0]));
    bitboard_t move_dst_left = move_left & dst;
    while (move_dst_left) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_left);
        AddPawnMoves<MoveBasicType::Simple, true, p>(
            dst_coord - (CURRENT_PAWN_MOVE_DELTA - 1), dst_coord, list, size);
    }
    const bitboard_t move_right =
        MoveAllPiecesByDelta<CURRENT_PAWN_MOVE_DELTA + 1>(src & (~FILE_BITBOARD[BOARD_SIDE - 1]));
    bitboard_t move_dst_right = move_right & dst;
    while (move_dst_right) {
        const coord_t dst_coord = q_util::ExtractLowestBit(move_dst_right);
        AddPawnMoves<MoveBasicType::Simple, true, p>(
            dst_coord - (CURRENT_PAWN_MOVE_DELTA + 1), dst_coord, list, size);
    }
    if constexpr (!p) {
        Q_ASSERT(IsCoordValidAndDefined(board.en_passant_coord));
        if (Q_UNLIKELY(board.en_passant_coord != NO_ENPASSANT_COORD)) {
            move_dst_left = move_left & MakeBitboardFromCoord(board.en_passant_coord);
            if (Q_UNLIKELY(move_dst_left)) {
                const coord_t dst_coord = board.en_passant_coord;
                AddPawnMoves<MoveBasicType::EnPassant, true, false>(
                    dst_coord - (CURRENT_PAWN_MOVE_DELTA - 1), dst_coord, list, size);
            }
            move_dst_right = move_right & MakeBitboardFromCoord(board.en_passant_coord);
            if (Q_UNLIKELY(move_dst_right)) {
                const coord_t dst_coord = board.en_passant_coord;
                AddPawnMoves<MoveBasicType::EnPassant, true, false>(
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
    GeneratePawnSimpleMoves<c, p>(list, src, dst, size);
    if constexpr (!p) {
        GeneratePawnDoubleMoves<c>(list, src & RANK_BITBOARD[GetPawnDoubleMoveRank(c)], dst, size);
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
            (pp == PromotionPolicy::None ? (~RANK_BITBOARD[GetPawnPromotionRank(c)])
                                         : RANK_BITBOARD[GetPawnPromotionRank(c)]);
        if constexpr (cp != CapturePolicy::OnlyCaptures) {
            GenerateAllPawnSimpleMoves<c, pp != PromotionPolicy::None>(board, list, src, size);
        }
        if constexpr (cp != CapturePolicy::None) {
            GenerateAllPawnCaptures<c, pp != PromotionPolicy::None>(board, list, src, size);
        }
    }
}

constexpr Move WHITE_KINGSIDE_CASTLING_MOVE =
    ConstructMove(WHITE_KING_INITIAL_POSITION, WHITE_KING_INITIAL_POSITION + 2,
                  GetMoveType<MoveBasicType::Castling>(false));
constexpr Move WHITE_QUEENSIDE_CASTLING_MOVE =
    ConstructMove(WHITE_KING_INITIAL_POSITION, WHITE_KING_INITIAL_POSITION - 2,
                  GetMoveType<MoveBasicType::Castling>(false));
constexpr Move BLACK_KINGSIDE_CASTLING_MOVE =
    ConstructMove(BLACK_KING_INITIAL_POSITION, BLACK_KING_INITIAL_POSITION + 2,
                  GetMoveType<MoveBasicType::Castling>(false));
constexpr Move BLACK_QUEENSIDE_CASTLING_MOVE =
    ConstructMove(BLACK_KING_INITIAL_POSITION, BLACK_KING_INITIAL_POSITION - 2,
                  GetMoveType<MoveBasicType::Castling>(false));
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
                list[size++] =
                    ConstructMove(src_coord, dst_coord, GetMoveType<MoveBasicType::Simple>(true));
            }
        }
        if constexpr (cp != CapturePolicy::OnlyCaptures) {
            bitboard_t move_dst = move_dst_init & board.bb_pieces[EMPTY_CELL];
            while (move_dst) {
                const coord_t dst_coord = q_util::ExtractLowestBit(move_dst);
                list[size++] =
                    ConstructMove(src_coord, dst_coord, GetMoveType<MoveBasicType::Simple>(false));
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

template <CapturePolicy cp, PromotionPolicy pp>
void GenerateMoves(const Board& board, MoveList& list) {
    Q_ASSERT(board.IsValid());
    size_t size = list.size;
    if (board.move_side == q_core::Color::White) {
        q_core::GenerateAllPawnMoves<q_core::Color::White, cp, pp>(board, list.moves, size);
        if constexpr (pp != PromotionPolicy::OnlyPromotions) {
            q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Knight, cp>(
                board, list.moves, size);
            q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Bishop, cp>(
                board, list.moves, size);
            q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::Rook, cp>(board, list.moves,
                                                                                 size);
            q_core::GenerateAllKNBRQMoves<q_core::Color::White, Piece::King, cp>(board, list.moves,
                                                                                 size);
        }
    } else {
        q_core::GenerateAllPawnMoves<q_core::Color::Black, cp, pp>(board, list.moves, size);
        if constexpr (pp != PromotionPolicy::OnlyPromotions) {
            q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Knight, cp>(
                board, list.moves, size);
            q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Bishop, cp>(
                board, list.moves, size);
            q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::Rook, cp>(board, list.moves,
                                                                                 size);
            q_core::GenerateAllKNBRQMoves<q_core::Color::Black, Piece::King, cp>(board, list.moves,
                                                                                 size);
        }
    }
    list.size = size;
}

void GenerateAllMoves(const Board& board, MoveList& list) {
    GenerateMoves<CapturePolicy::All, PromotionPolicy::All>(board, list);
}

void GenerateAllCaptures(const Board& board, MoveList& list) {
    GenerateMoves<CapturePolicy::OnlyCaptures, PromotionPolicy::All>(board, list);
}

void GenerateAllPromotions(const Board& board, MoveList& list) {
    GenerateMoves<CapturePolicy::None, PromotionPolicy::OnlyPromotions>(board, list);
}

void GenerateAllSimpleMoves(const Board& board, MoveList& list) {
    GenerateMoves<CapturePolicy::None, PromotionPolicy::None>(board, list);
}

// This function works only and only with moves, generated with movegen.cpp.
// May give incorrect answer, when move type is not simple, so uninitialized moves
// with non-simple type can be dangerous. Doesn't check fifty move rule flag
template <Color c>
bool IsMovePseudolegal(const Board& board, const Move move) {
    // for uninitialized and null moves
    if (move.src == move.dst) {
        return false;
    }
    if (board.cells[move.src] == EMPTY_CELL || GetCellColor(board.cells[move.src]) != c) {
        return false;
    }
    if (board.cells[move.dst] != EMPTY_CELL && GetCellColor(board.cells[move.dst]) == c) {
        return false;
    }
    const Piece piece = GetCellPiece(board.cells[move.src]);
    if (Q_UNLIKELY(IsMoveCastling(move))) {
        const CastlingSide castling_side = GetCastlingSide(move);
        Castling castling;
        bitboard_t castling_bitboard;
        if (castling_side == CastlingSide::Kingside) {
            castling = c == Color::White ? Castling::WhiteKingside : Castling::BlackKingside;
            castling_bitboard = c == Color::White ? WHITE_KINGSIDE_CASTLING_MOVE_BITBOARD
                                                  : BLACK_KINGSIDE_CASTLING_MOVE_BITBOARD;
        } else {
            castling = c == Color::White ? Castling::WhiteQueenside : Castling::BlackQueenside;
            castling_bitboard = c == Color::White ? WHITE_QUEENSIDE_CASTLING_MOVE_BITBOARD
                                                  : BLACK_QUEENSIDE_CASTLING_MOVE_BITBOARD;
        }
        return !((~board.bb_pieces[EMPTY_CELL]) & castling_bitboard) &&
               IsCastlingAllowed(board.castling, castling);
    }
    if (piece == Piece::Pawn) {
        constexpr int8_t CURRENT_PAWN_MOVE_DELTA = GetPawnMoveDelta(c);
        if (IsMovePawnDouble(move)) {
            const coord_t first_coord = move.src + CURRENT_PAWN_MOVE_DELTA;
            const coord_t second_coord = first_coord + CURRENT_PAWN_MOVE_DELTA;
            return q_util::CheckBit(board.bb_pieces[EMPTY_CELL], first_coord) &&
                   q_util::CheckBit(board.bb_pieces[EMPTY_CELL], second_coord);
        }
        if (Q_UNLIKELY(IsMoveEnPassant(move))) {
            if (board.en_passant_coord == NO_ENPASSANT_COORD) {
                return false;
            }
            const coord_t special_coord =
                board.en_passant_coord + GetPawnMoveDelta(GetInvertedColor(c));
            return (move.src == special_coord - 1 || move.src == special_coord + 1) &&
                   move.dst == board.en_passant_coord;
        }
        if (!IsMovePromotion(move) && GetRank(move.src) == GetPawnPromotionRank(c)) {
            return false;
        }
        if (board.cells[move.dst] == EMPTY_CELL) {
            return move.dst - move.src == CURRENT_PAWN_MOVE_DELTA;
        }
        return c == Color::White
                   ? q_util::CheckBit(WHITE_PAWN_REVERSED_ATTACK_BITBOARD[move.dst], move.src)
                   : q_util::CheckBit(BLACK_PAWN_REVERSED_ATTACK_BITBOARD[move.dst], move.src);
    }
    if (!IsMoveSimple(move)) {
        return false;
    }
    switch (piece) {
        case Piece::Knight: {
            return q_util::CheckBit(KNIGHT_ATTACK_BITBOARD[move.src], move.dst);
        }
        case Piece::Bishop: {
            return q_util::CheckBit(GetBishopAttackBitboard(~board.bb_pieces[EMPTY_CELL], move.src),
                                    move.dst);
        }
        case Piece::Rook: {
            return q_util::CheckBit(GetRookAttackBitboard(~board.bb_pieces[EMPTY_CELL], move.src),
                                    move.dst);
        }
        case Piece::Queen: {
            return q_util::CheckBit(GetBishopAttackBitboard(~board.bb_pieces[EMPTY_CELL], move.src),
                                    move.dst) ||
                   q_util::CheckBit(GetRookAttackBitboard(~board.bb_pieces[EMPTY_CELL], move.src),
                                    move.dst);
        }
        case Piece::King: {
            return q_util::CheckBit(KING_ATTACK_BITBOARD[move.src], move.dst);
        }
        default: {
            Q_UNREACHABLE();
        }
    }
    Q_UNREACHABLE();
}

bool IsMovePseudolegal(const Board& board, const Move move) {
    return board.move_side == Color::White ? IsMovePseudolegal<Color::White>(board, move)
                                           : IsMovePseudolegal<Color::Black>(board, move);
}

}  // namespace q_core
