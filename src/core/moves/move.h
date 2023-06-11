#ifndef QUIRKY_SRC_CORE_MOVES_MOVE_H
#define QUIRKY_SRC_CORE_MOVES_MOVE_H

#include "../board/types.h"

namespace q_core {

constexpr std::size_t MAX_NUMBER_OF_MOVES = 300;

constexpr uint8_t CAPTURE_MOVE_BIT = (1 << 0);
constexpr uint8_t PAWN_MOVE_BIT = (1 << 1);
constexpr uint8_t FIFTY_RULE_MOVE_BIT = (1 << 2);
constexpr uint8_t CASTLING_MOVE_BIT = (1 << 3);
constexpr uint8_t PROMOTION_MOVE_BIT = (1 << 4);
constexpr uint8_t NULL_MOVE_BIT = (1 << 5);
constexpr uint8_t KINGSIDE_CASTLING_MOVE = CASTLING_MOVE_BIT;
constexpr uint8_t QUEENSIDE_CASTLING_MOVE = CASTLING_MOVE_BIT + (1 << 6);
constexpr uint8_t KNIGHT_PROMOTION_MOVE = PROMOTION_MOVE_BIT;
constexpr uint8_t BISHOP_PROMOTION_MOVE = PROMOTION_MOVE_BIT + (1 << 6);
constexpr uint8_t ROOK_PROMOTION_MOVE = PROMOTION_MOVE_BIT + (1 << 6) * 2;
constexpr uint8_t QUEEN_PROMOTION_MOVE = PROMOTION_MOVE_BIT + (1 << 6) * 3;
constexpr uint8_t INVALID_MOVE = 255;

struct Move {
    coord_t src;
    coord_t dst;
    uint8_t type;
    int8_t info;
};

struct MoveList {
    Move moves[MAX_NUMBER_OF_MOVES];
    std::size_t size = 0;
};

inline constexpr bool IsMoveCapture(const Move move) {
    return static_cast<uint8_t>(move.type) & CAPTURE_MOVE_BIT;
}

inline constexpr bool IsMovePawnMove(const Move move) {
    return static_cast<uint8_t>(move.type) & PAWN_MOVE_BIT;
}

inline constexpr bool IsMoveFiftyRule(const Move move) {
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

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVE_H
