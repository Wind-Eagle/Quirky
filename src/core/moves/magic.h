#ifndef QUIRKY_SRC_CORE_MOVES_MAGIC_H
#define QUIRKY_SRC_CORE_MOVES_MAGIC_H

#include <array>

#include "../../util/bit.h"
#include "../../util/macro.h"
#include "../board/geometry.h"
#include "../board/types.h"
#include "move.h"

namespace {

inline constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE * 2>
GetPawnReversedAttackBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE * 2> res{};
    for (q_core::Color c : {q_core::Color::White, q_core::Color::Black}) {
        int8_t pawn_move_delta =
            (c == q_core::Color::White ? -q_core::BOARD_SIDE : q_core::BOARD_SIDE);
        for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
            if (q_core::IsCoordValidAndDefined(i + pawn_move_delta - 1)) {
                res[i + static_cast<uint8_t>(c) * q_core::BOARD_SIZE] |=
                    (1ULL << (i + pawn_move_delta - 1));
            }
            if (q_core::IsCoordValidAndDefined(i + pawn_move_delta + 1)) {
                res[i + static_cast<uint8_t>(c) * q_core::BOARD_SIZE] |=
                    (1ULL << (i + pawn_move_delta + 1));
            }
        }
    }
    return res;
}

inline constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetKnightAttackBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        for (q_core::subcoord_t dx = -2; dx <= 2; dx += 4) {
            for (q_core::subcoord_t dy = -1; dy <= 1; dy += 2) {
                q_core::subcoord_t x = q_core::GetRank(i);
                q_core::subcoord_t y = q_core::GetFile(i);
                if (q_core::IsSubcoordValid(x + dx) && q_core::IsSubcoordValid(y + dy)) {
                    res[i] |= (1ULL << q_core::MakeCoord(x + dx, y + dy));
                }
                if (q_core::IsSubcoordValid(x + dy) && q_core::IsSubcoordValid(y + dx)) {
                    res[i] |= (1ULL << q_core::MakeCoord(x + dy, y + dx));
                }
            }
        }
    }
    return res;
}

inline constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetKingAttackBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        for (q_core::subcoord_t dx = -1; dx <= 1; dx++) {
            for (q_core::subcoord_t dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                q_core::subcoord_t x = q_core::GetRank(i);
                q_core::subcoord_t y = q_core::GetFile(i);
                if (q_core::IsSubcoordValid(x + dx) && q_core::IsSubcoordValid(y + dy)) {
                    res[i] |= (1ULL << q_core::MakeCoord(x + dx, y + dy));
                }
            }
        }
    }
    return res;
}

}  // namespace

namespace q_core {

constexpr std::array<bitboard_t, BOARD_SIZE * 2> PAWN_REVERSED_ATTACK_BITBOARD =
    GetPawnReversedAttackBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> KNIGHT_ATTACK_BITBOARD = GetKnightAttackBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> KING_ATTACK_BITBOARD = GetKingAttackBitboard();

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_MOVES_MAGIC_H
