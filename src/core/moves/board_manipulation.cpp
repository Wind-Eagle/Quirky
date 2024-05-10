#include "board_manipulation.h"

#include "../board/hash.h"
#include "../util.h"
#include "attack.h"
#include "move.h"

namespace q_core {

constexpr bitboard_t WHITE_KINGSIDE_CASTLING_BITBOARD =
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION) |
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION + 3);
constexpr bitboard_t WHITE_QUEENSIDE_CASTLING_BITBOARD =
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION) |
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION - 4);
constexpr bitboard_t BLACK_KINGSIDE_CASTLING_BITBOARD =
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION) |
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION + 3);
constexpr bitboard_t BLACK_QUEENSIDE_CASTLING_BITBOARD =
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION) |
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION - 4);

constexpr bitboard_t TOTAL_CASTLING_CHANGE_BITBOARD =
    WHITE_KINGSIDE_CASTLING_BITBOARD | WHITE_QUEENSIDE_CASTLING_BITBOARD |
    BLACK_KINGSIDE_CASTLING_BITBOARD | BLACK_QUEENSIDE_CASTLING_BITBOARD;

inline std::array<Castling, 1ULL << q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD)>
GetCastlingChange() {
    std::array<Castling, 1ULL << q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD)> ans{};
    for (uint8_t submask = 0; submask < (1 << q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD));
         submask++) {
        bitboard_t mask = q_util::DepositBits(submask, TOTAL_CASTLING_CHANGE_BITBOARD);
        Castling castling = Castling::All;
        if (mask & WHITE_KINGSIDE_CASTLING_BITBOARD) {
            castling &= (~Castling::WhiteKingside);
        }
        if (mask & WHITE_QUEENSIDE_CASTLING_BITBOARD) {
            castling &= (~Castling::WhiteQueenside);
        }
        if (mask & BLACK_KINGSIDE_CASTLING_BITBOARD) {
            castling &= (~Castling::BlackKingside);
        }
        if (mask & BLACK_QUEENSIDE_CASTLING_BITBOARD) {
            castling &= (~Castling::BlackQueenside);
        }
        Q_ASSERT(submask < (1 << q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD)));
        Q_ASSERT(IsCastlingValid(castling));
        ans[submask] = castling;
    }
    return ans;
}

const std::array<Castling, 1ULL << q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD)>
    CASTLING_CHANGE = GetCastlingChange();

inline void UpdateCastling(Board &board, const bitboard_t change_bitboard) {
    if (Q_UNLIKELY(IsAnyCastlingAllowed(board.castling) &&
                   (change_bitboard & TOTAL_CASTLING_CHANGE_BITBOARD))) {
        board.hash ^= MakeZobristHashFromCastling(board.castling);
        uint8_t mask = q_util::ExtractBits(change_bitboard, TOTAL_CASTLING_CHANGE_BITBOARD);
        board.castling = CASTLING_CHANGE[mask] & board.castling;
        board.hash ^= MakeZobristHashFromCastling(board.castling);
    }
}

inline void BuildMakeMoveInfo(Board &board, const Move move, MakeMoveInfo &info) {
    info = MakeMoveInfo{board.cells[move.dst], board.hash, board.en_passant_coord, board.castling,
                        board.fifty_rule_move_count};
}

template <Color c, bool revert_cells>
void UnmakeMoveSimple(Board &board, const Move move, const cell_t src_cell, const cell_t dst_cell) {
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[src_cell] ^= change_bitboard;
    if (dst_cell != EMPTY_CELL) {
        board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] |= dst_bitboard;
    }
    board.bb_pieces[dst_cell] |= dst_bitboard;
    board.bb_pieces[EMPTY_CELL] ^= src_bitboard;
    if constexpr (revert_cells) {
        board.cells[move.src] = src_cell;
        board.cells[move.dst] = dst_cell;
    }
}

template <Color c, bool revert_cells>
void UnmakeMovePawnDouble(Board &board, const Move move) {
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
    board.bb_pieces[EMPTY_CELL] ^= change_bitboard;
    if constexpr (revert_cells) {
        board.cells[move.src] = MakeCell(c, Piece::Pawn);
        board.cells[move.dst] = EMPTY_CELL;
    }
}

template <Color c, bool revert_cells>
void UnmakeMoveEnPassant(Board &board, const Move move) {
    const coord_t taken_coord = (c == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t taken_bitboard = MakeBitboardFromCoord(taken_coord);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
    board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] ^= taken_bitboard;
    board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)] ^= taken_bitboard;
    board.bb_pieces[EMPTY_CELL] ^= src_bitboard | dst_bitboard | taken_bitboard;
    if constexpr (revert_cells) {
        board.cells[move.src] = MakeCell(c, Piece::Pawn);
        board.cells[move.dst] = EMPTY_CELL;
        board.cells[taken_coord] = MakeCell(GetInvertedColor(c), Piece::Pawn);
    }
}

template <Color c, bool revert_cells>
void UnmakeMoveCastling(Board &board, const Move move) {
    constexpr coord_t INITIAL_KING_POSITION =
        (c == Color::White ? WHITE_KING_INITIAL_POSITION : BLACK_KING_INITIAL_POSITION);
    if (GetCastlingSide(move) == CastlingSide::Kingside) {
        board.bb_pieces[MakeCell(c, Piece::King)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 2);
        board.bb_pieces[MakeCell(c, Piece::Rook)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 3) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 1);
        board.bb_colors[static_cast<uint8_t>(c)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 1) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 2) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 3);
        board.bb_pieces[EMPTY_CELL] ^= MakeBitboardFromCoord(INITIAL_KING_POSITION) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION + 1) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION + 2) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION + 3);
        if constexpr (revert_cells) {
            board.cells[INITIAL_KING_POSITION] = MakeCell(c, Piece::King);
            board.cells[INITIAL_KING_POSITION + 1] = EMPTY_CELL;
            board.cells[INITIAL_KING_POSITION + 2] = EMPTY_CELL;
            board.cells[INITIAL_KING_POSITION + 3] = MakeCell(c, Piece::Rook);
        }
    } else {
        board.bb_pieces[MakeCell(c, Piece::King)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 2);
        board.bb_pieces[MakeCell(c, Piece::Rook)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 4) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 1);
        board.bb_colors[static_cast<uint8_t>(c)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 1) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 2) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 4);
        board.bb_pieces[EMPTY_CELL] ^= MakeBitboardFromCoord(INITIAL_KING_POSITION) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION - 1) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION - 2) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION - 4);
        if constexpr (revert_cells) {
            board.cells[INITIAL_KING_POSITION] = MakeCell(c, Piece::King);
            board.cells[INITIAL_KING_POSITION - 2] = EMPTY_CELL;
            board.cells[INITIAL_KING_POSITION - 1] = EMPTY_CELL;
            board.cells[INITIAL_KING_POSITION - 4] = MakeCell(c, Piece::Rook);
        }
    }
}

template <Color c, bool revert_cells>
void UnmakeMovePromotion(Board &board, const Move move, const cell_t dst_cell) {
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    const cell_t promote_cell = MakeCell(c, GetPromotionPiece(move));
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= src_bitboard;
    board.bb_pieces[promote_cell] ^= dst_bitboard;
    if (dst_cell != EMPTY_CELL) {
        board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] |= dst_bitboard;
    }
    board.bb_pieces[dst_cell] |= dst_bitboard;
    board.bb_pieces[EMPTY_CELL] ^= src_bitboard;
    if constexpr (revert_cells) {
        board.cells[move.src] = MakeCell(c, Piece::Pawn);
        board.cells[move.dst] = dst_cell;
    }
}

template <Color c>
bool MakeMoveSimple(Board &board, const Move move, MakeMoveInfo &info) {
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    const cell_t src_cell = board.cells[move.src];
    const cell_t dst_cell = board.cells[move.dst];
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[src_cell] ^= change_bitboard;
    board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] &= ~dst_bitboard;
    board.bb_pieces[dst_cell] &= ~dst_bitboard;
    board.bb_pieces[EMPTY_CELL] |= src_bitboard;
    if (IsKingInCheck<c>(board)) {
        UnmakeMoveSimple<c, false>(board, move, src_cell, dst_cell);
        return false;
    }
    BuildMakeMoveInfo(board, move, info);
    board.hash ^= MakeZobristHashFromCell(move.src, src_cell) ^
                  MakeZobristHashFromCell(move.dst, src_cell) ^
                  MakeZobristHashFromCell(move.dst, dst_cell) ^
                  MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                  MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
    UpdateCastling(board, change_bitboard);
    if (Q_LIKELY(IsMoveFiftyRuleMove(move))) {
        board.fifty_rule_move_count = 0;
    } else {
        board.fifty_rule_move_count++;
    }
    board.en_passant_coord = NO_ENPASSANT_COORD;
    board.cells[move.src] = EMPTY_CELL;
    board.cells[move.dst] = src_cell;
    return true;
}

template <Color c>
bool MakeMovePawnDouble(Board &board, const Move move, MakeMoveInfo &info) {
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    const coord_t new_en_passant_coord =
        (c == Color::White ? move.src + BOARD_SIDE : move.src - BOARD_SIDE);
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
    board.bb_pieces[EMPTY_CELL] ^= change_bitboard;
    if (IsKingInCheck<c>(board)) {
        UnmakeMovePawnDouble<c, false>(board, move);
        return false;
    }
    BuildMakeMoveInfo(board, move, info);
    board.hash ^= MakeZobristHashFromCell(move.src, MakeCell(c, Piece::Pawn)) ^
                  MakeZobristHashFromCell(move.dst, MakeCell(c, Piece::Pawn)) ^
                  MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                  MakeZobristHashFromEnPassantCoord(new_en_passant_coord);
    board.fifty_rule_move_count = 0;
    board.en_passant_coord = new_en_passant_coord;
    board.cells[move.src] = EMPTY_CELL;
    board.cells[move.dst] = MakeCell(c, Piece::Pawn);
    return true;
}

template <Color c>
bool MakeMoveEnPassant(Board &board, const Move move, MakeMoveInfo &info) {
    const coord_t taken_coord = (c == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t taken_bitboard = MakeBitboardFromCoord(taken_coord);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
    board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] ^= taken_bitboard;
    board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)] ^= taken_bitboard;
    board.bb_pieces[EMPTY_CELL] ^= src_bitboard | dst_bitboard | taken_bitboard;
    if (IsKingInCheck<c>(board)) {
        UnmakeMoveEnPassant<c, false>(board, move);
        return false;
    }
    BuildMakeMoveInfo(board, move, info);
    board.hash ^= MakeZobristHashFromCell(move.src, MakeCell(c, Piece::Pawn)) ^
                  MakeZobristHashFromCell(move.dst, MakeCell(c, Piece::Pawn)) ^
                  MakeZobristHashFromCell(taken_coord, MakeCell(GetInvertedColor(c), Piece::Pawn)) ^
                  MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                  MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
    board.cells[move.src] = EMPTY_CELL;
    board.cells[move.dst] = MakeCell(c, Piece::Pawn);
    board.cells[taken_coord] = EMPTY_CELL;
    board.fifty_rule_move_count = 0;
    board.en_passant_coord = NO_ENPASSANT_COORD;
    return true;
}

template <Color c>
bool MakeMoveCastling(Board &board, const Move move, MakeMoveInfo &info) {
    constexpr coord_t INITIAL_KING_POSITION =
        (c == Color::White ? WHITE_KING_INITIAL_POSITION : BLACK_KING_INITIAL_POSITION);
    if (GetCastlingSide(move) == CastlingSide::Kingside) {
        // maybe it can be optimized
        if (IsCellAttacked<GetInvertedColor(c)>(board, INITIAL_KING_POSITION)) {
            return false;
        }
        if (IsCellAttacked<GetInvertedColor(c)>(board, INITIAL_KING_POSITION + 1)) {
            return false;
        }
        if (IsCellAttacked<GetInvertedColor(c)>(board, INITIAL_KING_POSITION + 2)) {
            return false;
        }
        board.bb_pieces[MakeCell(c, Piece::King)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 2);
        board.bb_pieces[MakeCell(c, Piece::Rook)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 3) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 1);
        board.bb_colors[static_cast<uint8_t>(c)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 1) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 2) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION + 3);
        board.bb_pieces[EMPTY_CELL] ^= MakeBitboardFromCoord(INITIAL_KING_POSITION) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION + 1) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION + 2) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION + 3);
        if (IsKingInCheck<c>(board)) {
            UnmakeMoveCastling<c, false>(board, move);
            return false;
        }
        BuildMakeMoveInfo(board, move, info);
        board.hash ^= MakeZobristHashFromCastling(board.castling);
        board.castling &= (~(c == Color::White ? Castling::WhiteAll : Castling::BlackAll));
        board.hash ^= MakeZobristHashFromCastling(board.castling);
        board.hash ^= MakeZobristHashFromCell(INITIAL_KING_POSITION, MakeCell(c, Piece::King)) ^
                      MakeZobristHashFromCell(INITIAL_KING_POSITION + 3, MakeCell(c, Piece::Rook)) ^
                      MakeZobristHashFromCell(INITIAL_KING_POSITION + 2, MakeCell(c, Piece::King)) ^
                      MakeZobristHashFromCell(INITIAL_KING_POSITION + 1, MakeCell(c, Piece::Rook)) ^
                      MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                      MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
        board.cells[INITIAL_KING_POSITION] = EMPTY_CELL;
        board.cells[INITIAL_KING_POSITION + 2] = MakeCell(c, Piece::King);
        board.cells[INITIAL_KING_POSITION + 1] = MakeCell(c, Piece::Rook);
        board.cells[INITIAL_KING_POSITION + 3] = EMPTY_CELL;
    } else {
        // maybe it can be optimized
        if (IsCellAttacked<GetInvertedColor(c)>(board, INITIAL_KING_POSITION)) {
            return false;
        }
        if (IsCellAttacked<GetInvertedColor(c)>(board, INITIAL_KING_POSITION - 1)) {
            return false;
        }
        if (IsCellAttacked<GetInvertedColor(c)>(board, INITIAL_KING_POSITION - 2)) {
            return false;
        }
        board.bb_pieces[MakeCell(c, Piece::King)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 2);
        board.bb_pieces[MakeCell(c, Piece::Rook)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 4) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 1);
        board.bb_colors[static_cast<uint8_t>(c)] ^=
            MakeBitboardFromCoord(INITIAL_KING_POSITION) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 1) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 2) |
            MakeBitboardFromCoord(INITIAL_KING_POSITION - 4);
        board.bb_pieces[EMPTY_CELL] ^= MakeBitboardFromCoord(INITIAL_KING_POSITION) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION - 1) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION - 2) |
                                       MakeBitboardFromCoord(INITIAL_KING_POSITION - 4);
        BuildMakeMoveInfo(board, move, info);
        board.hash ^= MakeZobristHashFromCastling(board.castling);
        board.castling &= (~(c == Color::White ? Castling::WhiteAll : Castling::BlackAll));
        board.hash ^= MakeZobristHashFromCastling(board.castling);
        board.hash ^= MakeZobristHashFromCell(INITIAL_KING_POSITION, MakeCell(c, Piece::King)) ^
                      MakeZobristHashFromCell(INITIAL_KING_POSITION - 4, MakeCell(c, Piece::Rook)) ^
                      MakeZobristHashFromCell(INITIAL_KING_POSITION - 2, MakeCell(c, Piece::King)) ^
                      MakeZobristHashFromCell(INITIAL_KING_POSITION - 1, MakeCell(c, Piece::Rook)) ^
                      MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                      MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
        board.cells[INITIAL_KING_POSITION] = EMPTY_CELL;
        board.cells[INITIAL_KING_POSITION - 2] = MakeCell(c, Piece::King);
        board.cells[INITIAL_KING_POSITION - 1] = MakeCell(c, Piece::Rook);
        board.cells[INITIAL_KING_POSITION - 4] = EMPTY_CELL;
    }
    board.fifty_rule_move_count++;
    board.en_passant_coord = NO_ENPASSANT_COORD;
    return true;
}

template <Color c>
bool MakeMovePromotion(Board &board, const Move move, MakeMoveInfo &info) {
    const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
    const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
    const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
    const cell_t dst_cell = board.cells[move.dst];
    const cell_t promote_cell = MakeCell(c, GetPromotionPiece(move));
    board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
    board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= src_bitboard;
    board.bb_pieces[promote_cell] ^= dst_bitboard;
    board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] &= ~dst_bitboard;
    board.bb_pieces[dst_cell] &= ~dst_bitboard;
    board.bb_pieces[EMPTY_CELL] |= src_bitboard;
    if (IsKingInCheck<c>(board)) {
        UnmakeMovePromotion<c, false>(board, move, dst_cell);
        return false;
    }
    BuildMakeMoveInfo(board, move, info);
    board.hash ^= MakeZobristHashFromCell(move.src, MakeCell(c, Piece::Pawn)) ^
                  MakeZobristHashFromCell(move.dst, promote_cell) ^
                  MakeZobristHashFromCell(move.dst, dst_cell) ^
                  MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                  MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
    UpdateCastling(board, change_bitboard);
    board.fifty_rule_move_count = 0;
    board.en_passant_coord = NO_ENPASSANT_COORD;
    board.cells[move.src] = EMPTY_CELL;
    board.cells[move.dst] = promote_cell;
    return true;
}

template <Color c>
bool MakeMove(Board &board, const Move move, MakeMoveInfo &info) {
    Q_ASSERT(board.IsValid());
    Q_ASSERT(!IsMoveNull(move) && !IsMoveUndefined(move));
    Q_ASSERT(c == board.move_side);
    const MoveBasicType move_basic_type = GetMoveBasicType(move);
    bool legal;
    switch (move_basic_type) {
        case MoveBasicType::Simple: {
            legal = MakeMoveSimple<c>(board, move, info);
            break;
        }
        case MoveBasicType::PawnDouble: {
            legal = MakeMovePawnDouble<c>(board, move, info);
            break;
        }
        case MoveBasicType::EnPassant: {
            legal = MakeMoveEnPassant<c>(board, move, info);
            break;
        }
        case MoveBasicType::Castling: {
            legal = MakeMoveCastling<c>(board, move, info);
            break;
        }
        case MoveBasicType::KnightPromotion:
        case MoveBasicType::BishopPromotion:
        case MoveBasicType::RookPromotion:
        case MoveBasicType::QueenPromotion: {
            legal = MakeMovePromotion<c>(board, move, info);
            break;
        }
        default:
            Q_UNREACHABLE();
    }
    if (!legal) {
        return false;
    }
    board.move_count++;
    board.move_side = GetInvertedColor(board.move_side);
    board.hash ^= ZOBRIST_HASH_MOVE_SIDE[0] ^ ZOBRIST_HASH_MOVE_SIDE[1];
    Q_ASSERT(board.IsValid());
    return true;
}

template <Color c>
bool UnmakeMove(Board &board, const Move move, const MakeMoveInfo &info) {
    Q_ASSERT(board.IsValid());
    Q_ASSERT(!IsMoveNull(move) && !IsMoveUndefined(move));
    Q_ASSERT(c != board.move_side);
    const MoveBasicType move_basic_type = GetMoveBasicType(move);
    board.move_count--;
    board.move_side = GetInvertedColor(board.move_side);
    board.hash = info.hash;
    board.en_passant_coord = info.en_passant;
    board.castling = info.castling;
    board.fifty_rule_move_count = info.fifty_rule_move_counter;
    switch (move_basic_type) {
        [[likely]] case MoveBasicType::Simple : {
            UnmakeMoveSimple<c, true>(board, move, board.cells[move.dst], info.dst_cell);
            break;
        }
        case MoveBasicType::PawnDouble: {
            UnmakeMovePawnDouble<c, true>(board, move);
            break;
        }
        [[unlikely]] case MoveBasicType::EnPassant : {
            UnmakeMoveEnPassant<c, true>(board, move);
            break;
        }
        [[unlikely]] case MoveBasicType::Castling : {
            UnmakeMoveCastling<c, true>(board, move);
            break;
        }
        [[unlikely]] case MoveBasicType::KnightPromotion:
        [[unlikely]] case MoveBasicType::BishopPromotion:
        [[unlikely]] case MoveBasicType::RookPromotion:
        [[unlikely]] case MoveBasicType::QueenPromotion : {
            UnmakeMovePromotion<c, true>(board, move, info.dst_cell);
            break;
        }
        default:
            Q_UNREACHABLE();
    }
    Q_ASSERT(board.IsValid());
    return true;
}

bool MakeMove(Board &board, const Move move, MakeMoveInfo &info) {
    if (board.move_side == Color::White) {
        return MakeMove<Color::White>(board, move, info);
    }
    return MakeMove<Color::Black>(board, move, info);
}

bool TryMakeMove(Board &board, const Move move, MakeMoveInfo &info) {
    return MakeMove(board, move, info);
}

void UnmakeMove(Board &board, const Move move, const MakeMoveInfo &info) {
    if (board.move_side == Color::White) {
        UnmakeMove<Color::Black>(board, move, info);
    } else {
        UnmakeMove<Color::White>(board, move, info);
    }
}

}  // namespace q_core
