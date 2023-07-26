#include "move.h"

#include "../util.h"

namespace q_core {

inline constexpr Piece GetPromotionPiece(const Move move) {
    Q_ASSERT(IsMovePromotion(move));
    uint8_t res = ((static_cast<uint8_t>(move.type) >> (PROMOTION_MOVE_BIT_LOG + 1)) &
                  ((1 << PROMOTION_MOVE_COUNT_LOG) - 1)) + static_cast<uint8_t>(Piece::Knight);
    Q_ASSERT(res >= static_cast<uint8_t>(Piece::Knight) && res <= static_cast<uint8_t>(Piece::Queen));
    return static_cast<Piece>(res);
}

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

}  // namespace q_core