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
    return Move{.src = src,
                .dst = dst,
                .type = GetPromotionMoveType(piece)};
}

Move TranslateStringToMove(const Board& board, const std::string_view& str) {
    Q_ASSERT(board.IsValid());
    Q_ASSERT(str.size() >= 4);
    Q_ASSERT(str.size() <= 5);
    coord_t src = CastStringToCoord(str.substr(0, 2));
    Q_ASSERT(IsCoordValidAndDefined(src));
    coord_t dst = CastStringToCoord(str.substr(2, 2));
    Q_ASSERT(IsCoordValidAndDefined(dst));
    Q_ASSERT(board.cells[src] != EMPTY_CELL);
    bool is_move_fifty_rule = false;
    if (q_core::GetCellPiece(board.cells[src]) == Piece::Pawn) {
        is_move_fifty_rule = true;
        if (dst - src == GetPawnMoveDelta<Color::White>() * 2 ||
            dst - src == GetPawnMoveDelta<Color::Black>() * 2) {
            return Move{.src = src, .dst = dst, .type = GetMoveType<MoveBasicType::PawnDouble>()};
        }
        if (dst - src != GetPawnMoveDelta<Color::White>() &&
            dst - src != GetPawnMoveDelta<Color::Black>()) {
            if (board.cells[dst] == EMPTY_CELL) {
                return Move{.src = src,
                            .dst = dst,
                            .type = GetMoveType<MoveBasicType::EnPassant>()};
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
    return Move{.src = src, .dst = dst, .type = GetMoveType<MoveBasicType::Simple>(is_move_fifty_rule)};
}

}  // namespace q_core
