#ifndef QUIRKY_SRC_CORE_MOVES_MOVE_H
#define QUIRKY_SRC_CORE_MOVES_MOVE_H

#include "../board/board.h"
#include "../board/geometry.h"
#include "util/bit.h"

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
inline constexpr uint8_t CAPTURE_MOVE_BIT = 8;
inline constexpr uint8_t MOVE_TYPE_BYTES_COUNT = 4;

inline constexpr Move ConstructMove(const coord_t src, const coord_t dst, const uint8_t type) {
    return Move{.src = src, .dst = dst, .type = type, .info = 0};
}

inline constexpr Move NULL_MOVE = ConstructMove(0, 0, 0);

inline constexpr MoveBasicType GetMoveBasicType(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return static_cast<MoveBasicType>(move.type & BASIC_TYPE_MOVE_MASK);
}

template <MoveBasicType move_basic_type>
inline constexpr uint8_t GetMoveType(bool is_move_capture) {
    return static_cast<uint8_t>(move_basic_type) + (is_move_capture ? CAPTURE_MOVE_BIT : 0);
}

inline constexpr bool operator==(const Move lhs, const Move rhs) {
    return lhs.src + (lhs.dst << 8) + (lhs.type << 16) ==
           rhs.src + (rhs.dst << 8) + (rhs.type << 16);
}

inline constexpr bool IsMoveNull(const Move move) {
    Q_ASSERT(move.type < (1 << MOVE_TYPE_BYTES_COUNT));
    return move.src == 0 && move.dst == 0;
}

inline constexpr bool IsMoveCapture(const Move move) {
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
    return static_cast<uint8_t>(GetMoveBasicType(move)) >=
           static_cast<uint8_t>(MoveBasicType::KnightPromotion);
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
    uint8_t res = static_cast<uint8_t>(move.type & BASIC_TYPE_MOVE_MASK) -
                  static_cast<uint8_t>(MoveBasicType::KnightPromotion) +
                  static_cast<uint8_t>(Piece::Knight);
    return static_cast<Piece>(res);
}

inline constexpr uint8_t PAWN_MOVE_DELTA = BOARD_SIDE;

inline constexpr int8_t GetPawnMoveDelta(Color c) {
    return c == Color::White ? PAWN_MOVE_DELTA : -PAWN_MOVE_DELTA;
}

inline constexpr uint8_t GetPawnPromotionRank(const Color c) {
    return c == Color::White ? BOARD_SIDE - 2 : 1;
}

inline constexpr bitboard_t GetPawnDoubleMoveRank(const Color c) {
    return c == Color::White ? 1 : BOARD_SIDE - 2;
}

inline constexpr uint8_t GetCastlingRank(const Color c) {
    return c == Color::White ? 0 : BOARD_SIDE - 1;
}

std::string CastMoveToString(Move move);
bool IsStringMoveWellFormated(const Board& board, const std::string_view& str);
Move TranslateStringToMove(const Board& board, const std::string_view& str);

bool IsMoveWellFormed(Move move, Color c);

using compressed_move_t = uint16_t;

inline constexpr Move GetDecompressedMove(const compressed_move_t compressed_move) {
    return ConstructMove(
        static_cast<coord_t>(compressed_move & (BOARD_SIZE - 1)),
        static_cast<coord_t>((compressed_move >> BOARD_SIZE_LOG) & (BOARD_SIZE - 1)),
        static_cast<uint8_t>((compressed_move >> (BOARD_SIZE_LOG * 2))));
}

inline constexpr compressed_move_t GetCompressedMove(const Move move) {
    return static_cast<uint16_t>(move.src) | (static_cast<uint16_t>(move.dst) << 6) |
           (static_cast<uint16_t>(move.type) << 12);
}

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MOVE_H
