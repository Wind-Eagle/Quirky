#ifndef QUIRKY_SRC_CORE_MOVES_MOVE_H
#define QUIRKY_SRC_CORE_MOVES_MOVE_H

#include "../board/types.h"
#include "../util.h"

namespace q_core {

constexpr std::size_t MAX_MOVES_COUNT = 300;

constexpr uint8_t CAPTURE_MOVE_BIT = (1 << 0);
constexpr uint8_t PAWN_DOUBLE_MOVE_BIT = (1 << 1);
constexpr uint8_t FIFTY_RULE_MOVE_BIT = (1 << 2);
constexpr uint8_t CASTLING_MOVE_BIT = (1 << 3);
constexpr uint8_t NULL_MOVE_BIT = (1 << 4);
constexpr uint8_t PROMOTION_MOVE_BIT = (1 << 5);
constexpr uint8_t KINGSIDE_CASTLING_MOVE = CASTLING_MOVE_BIT;
constexpr uint8_t QUEENSIDE_CASTLING_MOVE = CASTLING_MOVE_BIT + (1 << 6);
constexpr uint8_t KNIGHT_PROMOTION_MOVE = PROMOTION_MOVE_BIT;
constexpr uint8_t BISHOP_PROMOTION_MOVE = KNIGHT_PROMOTION_MOVE + (1 << 6);
constexpr uint8_t ROOK_PROMOTION_MOVE = KNIGHT_PROMOTION_MOVE + (1 << 6) * 2;
constexpr uint8_t QUEEN_PROMOTION_MOVE = KNIGHT_PROMOTION_MOVE + (1 << 6) * 3;
constexpr uint8_t INVALID_MOVE = 255;

constexpr uint8_t PROMOTION_MOVE_BIT_LOG = 5;
constexpr uint8_t PROMOTION_MOVE_COUNT_LOG = 2;

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

inline constexpr bool IsMoveCastling(const Move move) {
    return static_cast<uint8_t>(move.type) & CASTLING_MOVE_BIT;
}

inline constexpr bool IsMovePromotion(const Move move) {
    return static_cast<uint8_t>(move.type) & PROMOTION_MOVE_BIT;
}

inline constexpr bool IsMoveNull(const Move move) {
    return static_cast<uint8_t>(move.type) & NULL_MOVE_BIT;
}

inline constexpr bool IsMoveInvalid(const Move move) {
    return static_cast<uint8_t>(move.type) == INVALID_MOVE;
}

inline constexpr Piece GetPromotionPiece(const Move move) {
    uint8_t res = static_cast<uint8_t>(move.type) >> PROMOTION_MOVE_BIT_LOG &
                  ((1 << PROMOTION_MOVE_COUNT_LOG) - 1) + 1;
    Q_ASSERT(res >= static_cast<uint8_t>(Piece::Pawn) && res <= static_cast<uint8_t>(Piece::Queen));
    return static_cast<Piece>(res);
}

template <Color c>
inline constexpr cell_t GetPromotionCell(const Move move) {
    return MakeCell(c, GetPromotionPiece(move));
}

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVE_H
