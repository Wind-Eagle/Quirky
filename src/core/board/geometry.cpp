#include "geometry.h"

#include "core/board/types.h"

namespace q_core {

constexpr std::array<std::array<bitboard_t, BOARD_SIZE>, BOARD_SIZE> GenerateBitboardBetween() {
    std::array<std::array<bitboard_t, BOARD_SIZE>, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        for (coord_t j = 0; j < BOARD_SIZE; j++) {
            res[i][j] = FULL_BITBOARD;
        }
        const subcoord_t x = GetRank(i);
        const subcoord_t y = GetFile(i);
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                bitboard_t ans = MakeBitboardFromCoord(i);
                res[i][i] = ans;
                subcoord_t cur_x = x + dx;
                subcoord_t cur_y = y + dy;
                while (IsSubcoordValid(cur_x) && IsSubcoordValid(cur_y)) {
                    ans |= MakeBitboardFromCoord(MakeCoord(cur_x, cur_y));
                    res[i][MakeCoord(cur_x, cur_y)] = ans;
                    cur_x += dx;
                    cur_y += dy;
                }
            }
        }
    }
    return res;
}

inline constexpr std::array<std::array<bitboard_t, BOARD_SIZE>, BOARD_SIZE> BITBOARD_BETWEEN =
    GenerateBitboardBetween();

bitboard_t GetBitboardBetween(coord_t src, coord_t dst) { return BITBOARD_BETWEEN[src][dst]; }

}  // namespace q_core
