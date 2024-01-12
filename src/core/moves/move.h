#ifndef QUIRKY_SRC_CORE_MOVES_MOVE_H
#define QUIRKY_SRC_CORE_MOVES_MOVE_H

#include "../board/board.h"
#include "../board/types.h"

namespace q_core {

inline constexpr std::size_t MAX_MOVES_COUNT = 300;

inline constexpr uint8_t PAWN_DOUBLE_MOVE_TYPE = 1;
inline constexpr uint8_t EN_PASSANT_MOVE_TYPE = 2;
inline constexpr uint8_t CASTLING_MOVE_TYPE = 3;
inline constexpr uint8_t PROMOTION_MOVE_TYPE = 4;
inline constexpr uint8_t BASIC_TYPE_MOVE_MASK = (1 << 3) - 1;

inline constexpr uint8_t FIFTY_RULE_MOVE_BIT_LOG = 4;
inline constexpr uint8_t CAPTURE_MOVE_BIT_LOG = 5;
inline constexpr uint8_t VARIANT_MOVE_BIT_LOG = 6;

inline constexpr uint8_t FIFTY_RULE_MOVE_BIT = (1 << 4);
inline constexpr uint8_t CAPTURE_MOVE_BIT = (1 << 5);
inline constexpr uint8_t VARIANT_MOVE_BIT = (1 << 6);

inline constexpr uint8_t KINGSIDE_CASTLING_MOVE_TYPE = CASTLING_MOVE_TYPE;
inline constexpr uint8_t QUEENSIDE_CASTLING_MOVE_TYPE = CASTLING_MOVE_TYPE + VARIANT_MOVE_BIT;
inline constexpr uint8_t KNIGHT_PROMOTION_MOVE_TYPE = PROMOTION_MOVE_TYPE;
inline constexpr uint8_t BISHOP_PROMOTION_MOVE_TYPE = KNIGHT_PROMOTION_MOVE_TYPE + VARIANT_MOVE_BIT;
inline constexpr uint8_t ROOK_PROMOTION_MOVE_TYPE = KNIGHT_PROMOTION_MOVE_TYPE + VARIANT_MOVE_BIT * 2;
inline constexpr uint8_t QUEEN_PROMOTION_MOVE_TYPE = KNIGHT_PROMOTION_MOVE_TYPE + VARIANT_MOVE_BIT * 3;
inline constexpr uint8_t NULL_MOVE_TYPE = VARIANT_MOVE_BIT;
inline constexpr uint8_t UNDEFINED_MOVE_TYPE = VARIANT_MOVE_BIT * 2;

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

enum class MoveBasicType: uint8_t {
    Simple = 0,
    PawnDouble = PAWN_DOUBLE_MOVE_TYPE,
    EnPassant = EN_PASSANT_MOVE_TYPE,
    Castling = CASTLING_MOVE_TYPE,
    Promotion = PROMOTION_MOVE_TYPE
};

inline constexpr MoveBasicType GetMoveBasicType(const Move move) {
    return static_cast<MoveBasicType>(move.type & BASIC_TYPE_MOVE_MASK);
}

inline constexpr bool IsMoveCapture(const Move move) {
    return move.type & CAPTURE_MOVE_BIT;
}

inline constexpr bool IsMoveFiftyRuleMove(const Move move) {
    return move.type & FIFTY_RULE_MOVE_BIT;
}

inline constexpr bool IsMoveNull(const Move move) {
    return move.type == NULL_MOVE_TYPE;
}

inline constexpr bool IsMoveUndefined(const Move move) {
    return move.type == UNDEFINED_MOVE_TYPE;
}

inline constexpr bool IsMoveSimple(const Move move) {
    return GetMoveBasicType(move) == MoveBasicType::Simple;
}

inline constexpr bool IsMovePawnDouble(const Move move) {
    return GetMoveBasicType(move) == MoveBasicType::PawnDouble;
}

inline constexpr bool IsMoveEnPassant(const Move move) {
    return GetMoveBasicType(move) == MoveBasicType::EnPassant;
}

inline constexpr bool IsMoveCastling(const Move move) {
    return GetMoveBasicType(move) == MoveBasicType::Castling;
}

inline constexpr bool IsMovePromotion(const Move move) {
    return GetMoveBasicType(move) == MoveBasicType::Promotion;
}

inline Piece GetPromotionPiece(const Move move) {
    Q_ASSERT(IsMovePromotion(move));
    uint8_t res = ((move.type & (~(VARIANT_MOVE_BIT - 1))) >> VARIANT_MOVE_BIT_LOG) + static_cast<uint8_t>(Piece::Knight);
    Q_ASSERT(res >= static_cast<uint8_t>(Piece::Knight) &&
             res <= static_cast<uint8_t>(Piece::Queen));
    return static_cast<Piece>(res);
}

std::string CastMoveToString(Move move);

Move TranslateStringToMove(const Board& board, const std::string_view& str);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVE_H
