#include "magic_calc.h"

#include "../../util/bit.h"
#include "../../util/macro.h"
#include "../board/geometry.h"

namespace q_core {

std::array<bitboard_t, BOARD_SIZE * 2>
GetPawnReversedAttackBitboard() {
    std::array<bitboard_t, BOARD_SIZE * 2> res{};
    for (Color c : {Color::White, Color::Black}) {
        int8_t pawn_move_delta =
            (c == Color::White ? -BOARD_SIDE : BOARD_SIDE);
        for (coord_t i = 0; i < BOARD_SIZE; i++) {
            if (IsCoordValidAndDefined(i + pawn_move_delta - 1)) {
                res[i + static_cast<uint8_t>(c) * BOARD_SIZE] |=
                    (1ULL << (i + pawn_move_delta - 1));
            }
            if (IsCoordValidAndDefined(i + pawn_move_delta + 1)) {
                res[i + static_cast<uint8_t>(c) * BOARD_SIZE] |=
                    (1ULL << (i + pawn_move_delta + 1));
            }
        }
    }
    return res;
}

std::array<bitboard_t, BOARD_SIZE> GetKnightAttackBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        for (subcoord_t dx = -2; dx <= 2; dx += 4) {
            for (subcoord_t dy = -1; dy <= 1; dy += 2) {
                subcoord_t x = GetRank(i);
                subcoord_t y = GetFile(i);
                if (IsSubcoordValid(x + dx) && IsSubcoordValid(y + dy)) {
                    res[i] |= (1ULL << MakeCoord(x + dx, y + dy));
                }
                if (IsSubcoordValid(x + dy) && IsSubcoordValid(y + dx)) {
                    res[i] |= (1ULL << MakeCoord(x + dy, y + dx));
                }
            }
        }
    }
    return res;
}

std::array<bitboard_t, BOARD_SIZE> GetKingAttackBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        for (subcoord_t dx = -1; dx <= 1; dx++) {
            for (subcoord_t dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                subcoord_t x = GetRank(i);
                subcoord_t y = GetFile(i);
                if (IsSubcoordValid(x + dx) && IsSubcoordValid(y + dy)) {
                    res[i] |= (1ULL << MakeCoord(x + dx, y + dy));
                }
            }
        }
    }
    return res;
}

}  // namespace q_core