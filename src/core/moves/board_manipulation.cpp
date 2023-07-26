#include "board_manipulation.h"

#include "../board/hash.h"
#include "../util.h"

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

inline constexpr std::array<Castling, q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD) *
                                          (1 << NUMBER_OF_CASTLING_VARIANTS)>
GetCastlingChange() {
    std::array<Castling, q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD) *
                             (1 << NUMBER_OF_CASTLING_VARIANTS)>
        ans{};
    for (uint8_t submask = 0; submask < q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD);
         submask++) {
        bitboard_t mask = q_util::DepositBits(submask, TOTAL_CASTLING_CHANGE_BITBOARD);
        for (uint8_t castling_base = 0; castling_base < (1 << NUMBER_OF_CASTLING_VARIANTS);
             castling_base++) {
            Castling castling = static_cast<Castling>(castling_base);
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
            Q_ASSERT(submask < q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD));
            Q_ASSERT(IsCastlingValid(castling));
            ans[(submask << NUMBER_OF_CASTLING_VARIANTS) |
                static_cast<uint8_t>(castling_base)] = castling;
        }
    }
    return ans;
}

const std::array<Castling, q_util::GetBitCount(TOTAL_CASTLING_CHANGE_BITBOARD) *
                               (1 << NUMBER_OF_CASTLING_VARIANTS)>
    CASTLING_CHANGE = GetCastlingChange();

inline void UpdateCastling(Board& board, const bitboard_t change_bitboard) {
    if (Q_UNLIKELY(IsAnyCastlingAllowed(board.castling) &&
                   (change_bitboard & TOTAL_CASTLING_CHANGE_BITBOARD))) {
        board.hash ^= MakeZobristHashFromCastling(board.castling);
        uint8_t mask = q_util::ExtractBits(change_bitboard, TOTAL_CASTLING_CHANGE_BITBOARD);
        board.castling = CASTLING_CHANGE[(mask << NUMBER_OF_CASTLING_VARIANTS) |
                                         static_cast<uint8_t>(board.castling)];
        board.hash ^= MakeZobristHashFromCastling(board.castling);
    }
}

inline void BuildMakeMoveInfo(Board& board, const Move move, MakeMoveInfo& info) {
    info = MakeMoveInfo{.dst_cell = board.cells[move.dst],
                        .hash = board.hash,
                        .en_passant = board.en_passant_coord,
                        .castling = board.castling,
                        .fifty_rule_move_counter = board.fifty_rule_move_count};
}

template <Color c>
bool MakeMoveImpl(Board& board, const Move move, MakeMoveInfo& info) {
    Q_ASSERT(!IsMoveNull(move) && !IsMoveUndefined(move));
    Q_ASSERT(c == board.move_side);
    const uint8_t move_type = move.type & (PAWN_DOUBLE_MOVE_BIT & EN_PASSANT_MOVE_BIT &
                                           CASTLING_MOVE_BIT & PROMOTION_MOVE_BIT);
    switch (move_type) {
        [[likely]] case 0 : {
            const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
            const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
            const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
            const cell_t src_cell = board.cells[move.src];
            const cell_t dst_cell = board.cells[move.dst];
            board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
            board.bb_pieces[src_cell] ^= change_bitboard;
            board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] &= ~dst_bitboard;
            board.bb_pieces[dst_cell] &= ~dst_bitboard;
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
            break;
        }
        case PAWN_DOUBLE_MOVE_BIT: {
            const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
            const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
            const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
            const coord_t new_en_passant_coord =
                (c == Color::White ? move.src + BOARD_SIDE : move.src - BOARD_SIDE);
            board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
            board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
            BuildMakeMoveInfo(board, move, info);
            board.hash ^= MakeZobristHashFromCell(move.src, MakeCell(c, Piece::Pawn)) ^
                          MakeZobristHashFromCell(move.dst, MakeCell(c, Piece::Pawn)) ^
                          MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                          MakeZobristHashFromEnPassantCoord(new_en_passant_coord);
            board.fifty_rule_move_count = 0;
            board.en_passant_coord = new_en_passant_coord;
            board.cells[move.src] = EMPTY_CELL;
            board.cells[move.dst] = MakeCell(c, Piece::Pawn);
            break;
        }
            [[unlikely]] case EN_PASSANT_MOVE_BIT : {
                const coord_t taken_coord =
                    (c == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
                const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
                const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
                const bitboard_t taken_bitboard = MakeBitboardFromCoord(taken_coord);
                const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
                board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
                board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
                board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] ^= taken_bitboard;
                board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)] ^= taken_bitboard;
                BuildMakeMoveInfo(board, move, info);
                board.hash ^= MakeZobristHashFromCell(move.src, MakeCell(c, Piece::Pawn)) ^
                              MakeZobristHashFromCell(move.dst, MakeCell(c, Piece::Pawn)) ^
                              MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                              MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
                board.cells[move.src] = EMPTY_CELL;
                board.cells[move.dst] = MakeCell(c, Piece::Pawn);
                board.cells[taken_coord] = EMPTY_CELL;
                board.fifty_rule_move_count = 0;
                board.en_passant_coord = NO_ENPASSANT_COORD;
                break;
            }
            [[unlikely]] case CASTLING_MOVE_BIT : {
                constexpr coord_t INITIAL_KING_POSITION =
                    (c == Color::White ? WHITE_KING_INITIAL_POSITION : BLACK_KING_INITIAL_POSITION);
                if (move.type == KINGSIDE_CASTLING_MOVE) {
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
                    board.hash ^= MakeZobristHashFromCastling(board.castling);
                    board.castling &=
                        (~(c == Color::White ? Castling::WhiteAll : Castling::BlackAll));
                    board.hash ^= MakeZobristHashFromCastling(board.castling);
                    board.cells[INITIAL_KING_POSITION] = EMPTY_CELL;
                    board.cells[INITIAL_KING_POSITION + 1] = MakeCell(c, Piece::King);
                    board.cells[INITIAL_KING_POSITION + 2] = MakeCell(c, Piece::Rook);
                    board.cells[INITIAL_KING_POSITION + 3] = EMPTY_CELL;
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
                    board.hash ^= MakeZobristHashFromCastling(board.castling);
                    board.castling &=
                        (~(c == Color::White ? Castling::WhiteAll : Castling::BlackAll));
                    board.hash ^= MakeZobristHashFromCastling(board.castling);
                    board.cells[INITIAL_KING_POSITION] = EMPTY_CELL;
                    board.cells[INITIAL_KING_POSITION - 2] = MakeCell(c, Piece::King);
                    board.cells[INITIAL_KING_POSITION - 1] = MakeCell(c, Piece::Rook);
                    board.cells[INITIAL_KING_POSITION - 4] = EMPTY_CELL;
                }
                board.fifty_rule_move_count++;
                board.en_passant_coord = NO_ENPASSANT_COORD;
                break;
            }
            [[unlikely]] case PROMOTION_MOVE_BIT : {
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
                BuildMakeMoveInfo(board, move, info);
                board.hash ^= MakeZobristHashFromCell(move.src, MakeCell(c, Piece::Pawn)) ^
                              MakeZobristHashFromCell(move.dst, MakeCell(c, Piece::Pawn)) ^
                              MakeZobristHashFromCell(move.dst, dst_cell) ^
                              MakeZobristHashFromEnPassantCoord(board.en_passant_coord) ^
                              MakeZobristHashFromEnPassantCoord(NO_ENPASSANT_COORD);
                UpdateCastling(board, change_bitboard);
                board.fifty_rule_move_count = 0;
                board.en_passant_coord = NO_ENPASSANT_COORD;
                board.cells[move.src] = EMPTY_CELL;
                board.cells[move.dst] = promote_cell;
                break;
            }
        default:
            Q_UNREACHABLE();
    }
    board.move_count++;
    board.move_side = GetInvertedColor(board.move_side);
    board.hash ^= ZOBRIST_HASH_MOVE_SIDE[0] ^ ZOBRIST_HASH_MOVE_SIDE[1];
    Q_ASSERT(board.IsValid());
    return true;
}

template <Color c>
bool UnmakeMoveImpl(Board& board, const Move move, const MakeMoveInfo& info) {
    Q_ASSERT(!IsMoveNull(move) && !IsMoveUndefined(move));
    Q_ASSERT(c != board.move_side);
    const uint8_t move_type = move.type & (PAWN_DOUBLE_MOVE_BIT & EN_PASSANT_MOVE_BIT &
                                           CASTLING_MOVE_BIT & PROMOTION_MOVE_BIT);
    board.move_count--;
    board.move_side = GetInvertedColor(board.move_side);
    board.hash = info.hash;
    board.en_passant_coord = info.en_passant;
    board.castling = info.castling;
    board.fifty_rule_move_count = info.fifty_rule_move_counter;
    switch (move_type) {
        [[likely]] case 0 : {
            const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
            const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
            const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
            const cell_t src_cell = board.cells[move.dst];
            const cell_t dst_cell = info.dst_cell;
            board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
            board.bb_pieces[src_cell] ^= change_bitboard;
            if (dst_cell != EMPTY_CELL) {
                board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] |= src_bitboard;
            }
            board.bb_pieces[dst_cell] |= src_bitboard;
            board.cells[move.src] = src_cell;
            board.cells[move.dst] = dst_cell;
            break;
        }
        case PAWN_DOUBLE_MOVE_BIT: {
            const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
            const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
            const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
            const coord_t new_en_passant_coord =
                (c == Color::White ? move.src + BOARD_SIDE : move.src - BOARD_SIDE);
            board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
            board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
            board.cells[move.src] = MakeCell(c, Piece::Pawn);
            board.cells[move.dst] = EMPTY_CELL;
            break;
        }
            [[unlikely]] case EN_PASSANT_MOVE_BIT : {
                const coord_t taken_coord =
                    (c == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
                const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
                const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
                const bitboard_t taken_bitboard = MakeBitboardFromCoord(taken_coord);
                const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
                board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
                board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= change_bitboard;
                board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] ^= taken_bitboard;
                board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)] ^= taken_bitboard;
                board.cells[move.src] = MakeCell(c, Piece::Pawn);
                board.cells[move.dst] = EMPTY_CELL;
                board.cells[taken_coord] = MakeCell(GetInvertedColor(c), Piece::Pawn);
                break;
            }
            [[unlikely]] case CASTLING_MOVE_BIT : {
                constexpr coord_t INITIAL_KING_POSITION =
                    (c == Color::White ? WHITE_KING_INITIAL_POSITION : BLACK_KING_INITIAL_POSITION);
                if (move.type == KINGSIDE_CASTLING_MOVE) {
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
                    board.cells[INITIAL_KING_POSITION] = MakeCell(c, Piece::King);
                    board.cells[INITIAL_KING_POSITION + 1] = EMPTY_CELL;
                    board.cells[INITIAL_KING_POSITION + 2] = EMPTY_CELL;
                    board.cells[INITIAL_KING_POSITION + 3] = MakeCell(c, Piece::Rook);
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
                    board.cells[INITIAL_KING_POSITION] = MakeCell(c, Piece::King);
                    board.cells[INITIAL_KING_POSITION - 2] = EMPTY_CELL;
                    board.cells[INITIAL_KING_POSITION - 1] = EMPTY_CELL;
                    board.cells[INITIAL_KING_POSITION - 4] = MakeCell(c, Piece::Rook);
                }
                board.fifty_rule_move_count++;
                board.en_passant_coord = NO_ENPASSANT_COORD;
                break;
            }
            [[unlikely]] case PROMOTION_MOVE_BIT : {
                const bitboard_t src_bitboard = MakeBitboardFromCoord(move.src);
                const bitboard_t dst_bitboard = MakeBitboardFromCoord(move.dst);
                const bitboard_t change_bitboard = src_bitboard | dst_bitboard;
                const cell_t dst_cell = info.dst_cell;
                const cell_t promote_cell = MakeCell(c, GetPromotionPiece(move));
                board.bb_colors[static_cast<uint8_t>(c)] ^= change_bitboard;
                board.bb_pieces[MakeCell(c, Piece::Pawn)] ^= src_bitboard;
                board.bb_pieces[promote_cell] ^= dst_bitboard;
                if (dst_cell != EMPTY_CELL) {
                    board.bb_colors[static_cast<uint8_t>(GetInvertedColor(c))] |= dst_bitboard;
                }
                board.bb_pieces[dst_cell] |= dst_bitboard;
                board.cells[move.src] = MakeCell(c, Piece::Pawn);
                board.cells[move.dst] = dst_cell;
                break;
            }
        default:
            Q_UNREACHABLE();
    }
    Q_ASSERT(board.IsValid());
    return true;
}

bool MakeMove(Board& board, const Move move, MakeMoveInfo& info) {
    if (board.move_side == Color::White) {
        return MakeMoveImpl<Color::White>(board, move, info);
    }
    return MakeMoveImpl<Color::Black>(board, move, info);
}

void UnmakeMove(Board& board, const Move move, const MakeMoveInfo& info) {
    if (board.move_side == Color::White) {
        UnmakeMoveImpl<Color::Black>(board, move, info);
    }
    UnmakeMoveImpl<Color::White>(board, move, info);
}

}  // namespace q_core