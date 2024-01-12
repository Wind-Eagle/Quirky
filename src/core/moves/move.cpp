#include "move.h"

#include "../board/board.h"
#include "../util.h"

namespace q_core {

std::string CastMoveToString(const Move move) {
    if (IsMoveNull(move)) {
        return "null";
    }
    if (IsMoveUndefined(move)) {
        return "undefined";
    }
    std::string ans = CastCoordToString(move.src) + CastCoordToString(move.dst);
    if (IsMovePromotion(move)) {
        ans += CastPieceToChar(GetPromotionPiece(move));
    }
    return ans;
}

constexpr uint8_t PAWN_MOVE_DELTA = BOARD_SIDE;

template <Color c>
inline constexpr int8_t GetPawnMoveDelta() {
    return c == Color::White ? PAWN_MOVE_DELTA : -PAWN_MOVE_DELTA;
}

Move TranslatePromotionStringToMove(const Board& board, const std::string_view& str, const coord_t src, const coord_t dst) {
    Piece piece = CastCharToPiece(str[4]);
    if (!IsPieceValid(piece) || piece == Piece::Pawn || piece == Piece::King) {
        return Move{.type = UNDEFINED_MOVE_TYPE};
    }
    if (piece == Piece::Queen) {
        return Move{.src = src, .dst = dst, .type = QUEEN_PROMOTION_MOVE_TYPE};
    }
    if (piece == Piece::Rook) {
        return Move{.src = src, .dst = dst, .type = ROOK_PROMOTION_MOVE_TYPE};
    }
    if (piece == Piece::Knight) {
        return Move{.src = src, .dst = dst, .type = KNIGHT_PROMOTION_MOVE_TYPE};
    }
    return Move{.src = src, .dst = dst, .type = BISHOP_PROMOTION_MOVE_TYPE};
}

Move TranslateStringToMove(const Board& board, const std::string_view& str) {
    Q_ASSERT(board.IsValid());
    Q_ASSERT(str.size() >= 4);
    Q_ASSERT(str.size() <= 5);
    coord_t src = CastStringToCoord(str.substr(0, 2));
    if (!IsCoordValidAndDefined(src)) {
        return Move{.type = UNDEFINED_MOVE_TYPE};
    }
    coord_t dst = CastStringToCoord(str.substr(2, 2));
    if (!IsCoordValidAndDefined(dst)) {
        return Move{.type = UNDEFINED_MOVE_TYPE};
    }
    if (board.cells[src] == EMPTY_CELL) {
        return Move{.type = UNDEFINED_MOVE_TYPE};
    }
    uint8_t type = 0;
    if (q_core::GetCellPiece(board.cells[src]) == Piece::Pawn) {
        type |= FIFTY_RULE_MOVE_BIT;
        if (dst - src == GetPawnMoveDelta<Color::White>() * 2 || dst - src == GetPawnMoveDelta<Color::Black>() * 2) {
            return Move{.src = src, .dst = dst, .type = PAWN_DOUBLE_MOVE_TYPE};
        }
        if (dst - src != GetPawnMoveDelta<Color::White>() && dst - src != GetPawnMoveDelta<Color::Black>()) {
            if (board.cells[dst] == EMPTY_CELL) {
                return Move{.src = src, .dst = dst, .type = EN_PASSANT_MOVE_TYPE | CAPTURE_MOVE_BIT | FIFTY_RULE_MOVE_BIT};
            }
        }
        if (str.size() == 5) {
            return TranslatePromotionStringToMove(board, str, src, dst);
        }
    }
    if (q_core::GetCellPiece(board.cells[src]) == Piece::King) {
        if (GetFile(dst) - GetFile(src) == 2) {
            return Move{.src = src, .dst = dst, .type = KINGSIDE_CASTLING_MOVE_TYPE};
        }
        if (GetFile(dst) - GetFile(src) == -2) {
            return Move{.src = src, .dst = dst, .type = QUEENSIDE_CASTLING_MOVE_TYPE};
        }
    }
    if (board.cells[dst] != EMPTY_CELL) {
        type |= CAPTURE_MOVE_BIT | FIFTY_RULE_MOVE_BIT;
    }
    return Move{.src = src, .dst = dst, .type = type};
}

}  // namespace q_core