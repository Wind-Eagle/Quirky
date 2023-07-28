#ifndef QUIRKY_SRC_CORE_MOVES_MOVE_H
#define QUIRKY_SRC_CORE_MOVES_MOVE_H

#include "../board/board.h"
#include "../board/types.h"

namespace q_core {

constexpr std::size_t MAX_MOVES_COUNT = 300;

constexpr uint8_t FIFTY_RULE_MOVE_BIT = (1 << 0);
constexpr uint8_t CAPTURE_MOVE_BIT = (1 << 1);
constexpr uint8_t PAWN_DOUBLE_MOVE_BIT = (1 << 2);
constexpr uint8_t CASTLING_MOVE_BIT = (1 << 3);
constexpr uint8_t EN_PASSANT_MOVE_BIT = (1 << 4);
constexpr uint8_t PROMOTION_MOVE_BIT = (1 << 5);
constexpr uint8_t VARIANT_MOVE_BIT = (1 << 6);
constexpr uint8_t KINGSIDE_CASTLING_MOVE_TYPE = CASTLING_MOVE_BIT;
constexpr uint8_t QUEENSIDE_CASTLING_MOVE_TYPE = CASTLING_MOVE_BIT + VARIANT_MOVE_BIT;
constexpr uint8_t KNIGHT_PROMOTION_MOVE_TYPE = PROMOTION_MOVE_BIT;
constexpr uint8_t BISHOP_PROMOTION_MOVE_TYPE = KNIGHT_PROMOTION_MOVE_TYPE + VARIANT_MOVE_BIT;
constexpr uint8_t ROOK_PROMOTION_MOVE_TYPE = KNIGHT_PROMOTION_MOVE_TYPE + VARIANT_MOVE_BIT * 2;
constexpr uint8_t QUEEN_PROMOTION_MOVE_TYPE = KNIGHT_PROMOTION_MOVE_TYPE + VARIANT_MOVE_BIT * 3;
constexpr uint8_t NULL_MOVE_TYPE = VARIANT_MOVE_BIT;
constexpr uint8_t UNDEFINED_MOVE_TYPE = VARIANT_MOVE_BIT * 2;

constexpr uint8_t FIFTY_RULE_MOVE_BIT_LOG = 0;
constexpr uint8_t PROMOTION_MOVE_BIT_LOG = 5;
constexpr uint8_t PROMOTION_MOVE_COUNT_LOG = 2;
constexpr uint8_t VARIANT_MOVE_BIT_LOG = 6;

struct Move {
    coord_t src;
    coord_t dst;
    uint8_t type;
    int8_t info;
};

struct MoveList {
    Move moves[MAX_MOVES_COUNT];
    std::size_t size = 0;
};

inline constexpr bool IsMoveCapture(const Move move) {
    return static_cast<uint8_t>(move.type) & CAPTURE_MOVE_BIT;
}

inline constexpr bool IsMovePawnDoubleMove(const Move move) {
    return static_cast<uint8_t>(move.type) & PAWN_DOUBLE_MOVE_BIT;
}

inline constexpr bool IsMoveFiftyRuleMove(const Move move) {
    return static_cast<uint8_t>(move.type) & FIFTY_RULE_MOVE_BIT;
}

inline constexpr bool IsMoveEnPassant(const Move move) {
    return static_cast<uint8_t>(move.type) & EN_PASSANT_MOVE_BIT;
}

inline constexpr bool IsMoveCastling(const Move move) {
    return static_cast<uint8_t>(move.type) & CASTLING_MOVE_BIT;
}

inline constexpr bool IsMovePromotion(const Move move) {
    return static_cast<uint8_t>(move.type) & PROMOTION_MOVE_BIT;
}

inline constexpr bool IsMoveNull(const Move move) {
    return static_cast<uint8_t>(move.type) == NULL_MOVE_TYPE;
}

inline constexpr bool IsMoveUndefined(const Move move) {
    return static_cast<uint8_t>(move.type) == UNDEFINED_MOVE_TYPE;
}

inline Piece GetPromotionPiece(const Move move) {
    Q_ASSERT(IsMovePromotion(move));
    uint8_t res = ((static_cast<uint8_t>(move.type) >> (PROMOTION_MOVE_BIT_LOG + 1)) &
                   ((1 << PROMOTION_MOVE_COUNT_LOG) - 1)) +
                  static_cast<uint8_t>(Piece::Knight);
    Q_ASSERT(res >= static_cast<uint8_t>(Piece::Knight) &&
             res <= static_cast<uint8_t>(Piece::Queen));
    return static_cast<Piece>(res);
}

std::string CastMoveToString(Move move);

Move TranslateStringIntoMove(const Board& board, const std::string_view& str);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVE_H
