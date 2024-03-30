#ifndef QUIRKY_SRC_CORE_MOVES_MOVE_H
#define QUIRKY_SRC_CORE_MOVES_MOVE_H

#include "../board/board.h"
#include "../board/types.h"
#include "../util.h"

namespace q_core {

inline constexpr std::size_t MAX_MOVES_COUNT = 300;

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

enum class MoveBasicType : uint8_t {
    Simple = 0,
    PawnDouble = 1,
    EnPassant = 2,
    Castling = 3,
    KnightPromotion = 4,
    BishopPromotion = 5,
    RookPromotion = 6,
    QueenPromotion = 7
};

enum class CastlingSide : bool { Queenside = 0, Kingside = 1 };

inline constexpr uint8_t BASIC_TYPE_MOVE_MASK = 7;
inline constexpr uint8_t FIFTY_RULE_MOVE_BIT = 8;
inline constexpr uint8_t MOVE_TYPE_BYTES_COUNT = 4;

inline constexpr MoveBasicType GetMoveBasicType(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return static_cast<MoveBasicType>(move.type & BASIC_TYPE_MOVE_MASK);
}

template <MoveBasicType move_basic_type>
inline constexpr uint8_t GetMoveType() {
    Q_STATIC_ASSERT(move_basic_type != MoveBasicType::Simple);
    return static_cast<uint8_t>(move_basic_type) + (move_basic_type != MoveBasicType::Castling ? FIFTY_RULE_MOVE_BIT : 0);
}

template <MoveBasicType move_basic_type>
inline constexpr uint8_t GetMoveType(bool is_move_fifty_rule) {
    Q_STATIC_ASSERT(move_basic_type == MoveBasicType::Simple);
    return static_cast<uint8_t>(move_basic_type) + (is_move_fifty_rule ? FIFTY_RULE_MOVE_BIT : 0);
}

inline constexpr uint8_t GetPromotionMoveType(const Piece piece) {
    Q_ASSERT(!IsPieceValid(piece) || piece == Piece::Pawn || piece == Piece::King);
    return static_cast<uint8_t>(piece) - static_cast<uint8_t>(Piece::Knight) + GetMoveType<MoveBasicType::KnightPromotion>();
}

inline constexpr bool IsMoveFiftyRuleMove(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return move.type > BASIC_TYPE_MOVE_MASK;
}

inline constexpr bool IsMoveSimple(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return GetMoveBasicType(move) == MoveBasicType::Simple;
}

inline constexpr bool IsMovePawnDouble(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return GetMoveBasicType(move) == MoveBasicType::PawnDouble;
}

inline constexpr bool IsMoveEnPassant(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return GetMoveBasicType(move) == MoveBasicType::EnPassant;
}

inline constexpr bool IsMoveCastling(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return GetMoveBasicType(move) == MoveBasicType::Castling;
}

inline constexpr bool IsMovePromotion(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return static_cast<uint8_t>(move.type) >=
           static_cast<uint8_t>(GetMoveType<MoveBasicType::KnightPromotion>());
}

inline constexpr bitboard_t KINGSIDE_CASTLING_DST_BITBOARD =
    MakeBitboardFromCoord(WHITE_KING_INITIAL_POSITION + 2) |
    MakeBitboardFromCoord(BLACK_KING_INITIAL_POSITION + 2);

inline constexpr CastlingSide GetCastlingSide(const Move move) {
    Q_ASSERT(IsMoveCastling(move));
    return q_util::CheckBit(KINGSIDE_CASTLING_DST_BITBOARD, move.dst) ? CastlingSide::Kingside
                                                                      : CastlingSide::Queenside;
}

inline constexpr Piece GetPromotionPiece(const Move move) {
    Q_ASSERT(IsMovePromotion(move));
    uint8_t res = static_cast<uint8_t>(move.type) -
                  static_cast<uint8_t>(GetMoveType<MoveBasicType::KnightPromotion>()) +
                  static_cast<uint8_t>(Piece::Knight);
    return static_cast<Piece>(res);
}

inline constexpr uint8_t PAWN_MOVE_DELTA = BOARD_SIDE;

template <Color c>
inline constexpr int8_t GetPawnMoveDelta() {
    return c == Color::White ? PAWN_MOVE_DELTA : -PAWN_MOVE_DELTA;
}

std::string CastMoveToString(Move move);

Move TranslateStringToMove(const Board& board, const std::string_view& str);

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVE_H
