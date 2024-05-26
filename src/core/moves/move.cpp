#include "move.h"

#include "../board/board.h"
#include "../util.h"

namespace q_core {

std::string CastMoveToString(const Move move) {
    std::string ans = CastCoordToString(move.src) + CastCoordToString(move.dst);
    if (IsMovePromotion(move)) {
        ans += CastPieceToChar(GetPromotionPiece(move));
    }
    return ans;
}

Move TranslatePromotionStringToMove(const Board& board, const std::string_view& str,
                                    const coord_t src, const coord_t dst) {
    Piece piece = CastCharToPiece(str[4]);
    return Move{.src = src, .dst = dst, .type = GetPromotionMoveType(piece)};
}

bool IsStringMoveWellFormated(const Board& board, const std::string_view& str) {
    if (str.size() < 4 || str.size() > 5) {
        return false;
    }
    coord_t src = CastStringToCoord(str.substr(0, 2));
    coord_t dst = CastStringToCoord(str.substr(2, 2));
    if (!IsCoordValidAndDefined(src) || !IsCoordValidAndDefined(dst) || board.cells[src] == EMPTY_CELL) {
        return false;
    }
    if (str.size() == 5) {
        Piece piece = CastCharToPiece(str[4]);
        if (!IsPieceValid(piece) || piece == Piece::Pawn || piece == Piece::King) {
            return false;
        }
    }
    return true;
}

Move TranslateStringToMove(const Board& board, const std::string_view& str) {
    Q_ASSERT(board.IsValid());
    Q_ASSERT(IsStringMoveWellFormated(board, str));
    coord_t src = CastStringToCoord(str.substr(0, 2));
    coord_t dst = CastStringToCoord(str.substr(2, 2));
    bool is_move_fifty_rule = false;
    if (q_core::GetCellPiece(board.cells[src]) == Piece::Pawn) {
        is_move_fifty_rule = true;
        if (dst - src == GetPawnMoveDelta(Color::White) * 2 ||
            dst - src == GetPawnMoveDelta(Color::Black) * 2) {
            return Move{.src = src, .dst = dst, .type = GetMoveType<MoveBasicType::PawnDouble>()};
        }
        if (dst - src != GetPawnMoveDelta(Color::White) &&
            dst - src != GetPawnMoveDelta(Color::Black)) {
            if (board.cells[dst] == EMPTY_CELL) {
                return Move{
                    .src = src, .dst = dst, .type = GetMoveType<MoveBasicType::EnPassant>()};
            }
        }
        if (str.size() == 5) {
            return TranslatePromotionStringToMove(board, str, src, dst);
        }
    }
    if (q_core::GetCellPiece(board.cells[src]) == Piece::King) {
        if (GetFile(dst) - GetFile(src) == 2) {
            return Move{.src = src, .dst = dst, .type = GetMoveType<MoveBasicType::Castling>()};
        }
        if (GetFile(dst) - GetFile(src) == -2) {
            return Move{.src = src, .dst = dst, .type = GetMoveType<MoveBasicType::Castling>()};
        }
    }
    if (board.cells[dst] != EMPTY_CELL) {
        is_move_fifty_rule = true;
    }
    return Move{
        .src = src, .dst = dst, .type = GetMoveType<MoveBasicType::Simple>(is_move_fifty_rule)};
}

bool IsMoveWellFormed(Move move, Color c) {
    if (!IsCoordValidAndDefined(move.src) || !IsCoordValidAndDefined(move.dst)) {
        return false;
    }
    const MoveBasicType move_basic_type = GetMoveBasicType(move);
    switch (move_basic_type) {
        case MoveBasicType::Simple: {
            return true;
        }
        case MoveBasicType::PawnDouble: {
            const subcoord_t pawn_double_move_rank = GetPawnDoubleMoveRank(c);
            return GetRank(move.src) == pawn_double_move_rank &&
                   GetRank(move.dst) == pawn_double_move_rank + (c == Color::White ? 2 : -2) &&
                   GetFile(move.src) == GetFile(move.dst);
        }
        case MoveBasicType::EnPassant: {
            const subcoord_t en_passant_rank = GetPawnDoubleMoveRank(GetInvertedColor(c)) + (c == Color::White ? -2 : 2);
            return GetRank(move.src) == en_passant_rank &&
                   GetRank(move.dst) == en_passant_rank + (c == Color::White ? 1 : -1) &&
                   (GetFile(move.src) == GetFile(move.dst) + 1 ||
                   GetFile(move.src) + 1 == GetFile(move.dst));
        }
        case MoveBasicType::Castling: {
            const coord_t initial_king_position = c == Color::White ? WHITE_KING_INITIAL_POSITION : BLACK_KING_INITIAL_POSITION;
            return (move.src == initial_king_position && move.dst == initial_king_position + 2) ||
            (move.src == initial_king_position && move.dst == initial_king_position - 2);
        }
        case MoveBasicType::KnightPromotion:
        case MoveBasicType::BishopPromotion:
        case MoveBasicType::RookPromotion:
        case MoveBasicType::QueenPromotion: {
            if (GetRank(move.src) != GetPawnPromotionRank(c) || GetRank(move.dst) != GetPawnPromotionRank(c) + (c == Color::White ? 1 : -1)) {
                return false;
            }
            const subcoord_t src_file = GetFile(move.src);
            const subcoord_t dst_file = GetFile(move.dst);
            return src_file == dst_file || src_file + 1 == dst_file || src_file == dst_file + 1;
        }
        default:
            Q_UNREACHABLE();
    }
    Q_UNREACHABLE();
}

}  // namespace q_core
