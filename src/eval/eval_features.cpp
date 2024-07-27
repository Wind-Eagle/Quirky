#include "eval_features.h"

#include <array>

#include "../core/board/geometry.h"
#include "../core/moves/magic.h"
#include "../core/util.h"
#include "../util/math.h"
#include "model.h"

using namespace q_core;

namespace q_eval {

constexpr std::array<bitboard_t, BOARD_SIZE> GetWhitePawnFrontspanBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        for (subcoord_t j = rank + 1; j < BOARD_SIDE; j++) {
            res[i] |= MakeBitboardFromCoord(MakeCoord(j, file));
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> GetBlackPawnFrontspanBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        for (subcoord_t j = rank - 1; j >= 0; j--) {
            res[i] |= MakeBitboardFromCoord(MakeCoord(j, file));
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIDE> GetPawnNeighboursBitboard() {
    std::array<bitboard_t, BOARD_SIDE> res{};
    for (coord_t file = 0; file < BOARD_SIDE; file++) {
        if (file > 0) {
            res[file] |= FILE_BITBOARD[file - 1];
        }
        if (file < BOARD_SIDE - 1) {
            res[file] |= FILE_BITBOARD[file + 1];
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> WHITE_PAWN_FRONTSPAN_BITBOARD =
    GetWhitePawnFrontspanBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> BLACK_PAWN_FRONTSPAN_BITBOARD =
    GetBlackPawnFrontspanBitboard();
constexpr std::array<bitboard_t, BOARD_SIDE> PAWN_NEIGHBOURS_BITBOARD = GetPawnNeighboursBitboard();

bool IsPawnIsolated(const PawnContext& context) {
    return !(PAWN_NEIGHBOURS_BITBOARD[GetFile(context.pawn_coord)] & context.our_pawns);
}

bool IsPawnDoubled(const PawnContext& context) {
    const auto& colored_pawn_frontspan = context.color == Color::White
                                             ? WHITE_PAWN_FRONTSPAN_BITBOARD
                                             : BLACK_PAWN_FRONTSPAN_BITBOARD;
    return colored_pawn_frontspan[context.pawn_coord] & context.our_pawns;
}

}  // namespace q_eval