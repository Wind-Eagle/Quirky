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

constexpr std::array<bitboard_t, BOARD_SIZE> GetWhitePawnBackwardSentryBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = MakeCoord(1, 0); i < MakeCoord(BOARD_SIDE - 2, 0); i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        if (file > 0) {
            res[file] |= MakeBitboardFromCoord(MakeCoord(rank + 2, file - 1));
        }
        if (file < BOARD_SIDE - 1) {
            res[file] |= MakeBitboardFromCoord(MakeCoord(rank + 2, file + 1));
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> GetBlackPawnBackwardSentryBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = MakeCoord(2, 0); i < MakeCoord(BOARD_SIDE - 1, 0); i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        if (file > 0) {
            res[file] |= MakeBitboardFromCoord(MakeCoord(rank - 2, file - 1));
        }
        if (file < BOARD_SIDE - 1) {
            res[file] |= MakeBitboardFromCoord(MakeCoord(rank - 2, file + 1));
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> GetWhitePawnPassedBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        for (subcoord_t j = rank + 1; j < BOARD_SIDE; j++) {
            if (file > 0) {
                res[file] |= MakeBitboardFromCoord(MakeCoord(j, file - 1));
            }
            res[file] |= MakeBitboardFromCoord(MakeCoord(j, file));
            if (file < BOARD_SIDE - 1) {
                res[file] |= MakeBitboardFromCoord(MakeCoord(j, file + 1));
            }
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> GetBlackPawnPassedBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        for (subcoord_t j = rank - 1; j >= 0; j--) {
            if (file > 0) {
                res[file] |= MakeBitboardFromCoord(MakeCoord(j, file - 1));
            }
            res[file] |= MakeBitboardFromCoord(MakeCoord(j, file));
            if (file < BOARD_SIDE - 1) {
                res[file] |= MakeBitboardFromCoord(MakeCoord(j, file + 1));
            }
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> GetPawnConnectedBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        subcoord_t rank = GetRank(i);
        subcoord_t file = GetFile(i);
        if (file > 0) {
            res[file] |= MakeBitboardFromCoord(MakeCoord(rank, file - 1));
        }
        if (file < BOARD_SIDE - 1) {
            res[file] |= MakeBitboardFromCoord(MakeCoord(rank, file + 1));
        }
    }
    return res;
}

constexpr std::array<uint8_t, 1 << BOARD_SIDE> GetPawnIslandsCount() {
    std::array<uint8_t, 1 << BOARD_SIDE> res{};
    for (uint16_t mask = 0; mask < (1 << BOARD_SIDE); mask++) {
        uint8_t ans = 0;
        bool prev = false;
        for (uint8_t file = 0; file < BOARD_SIDE; file++) {
            bool cur = mask & (1 << file);
            if (!prev && cur) {
                ans++;
            }
            prev = cur;
        }
        res[mask] = ans;
    }
    return res;
};

constexpr std::array<bitboard_t, BOARD_SIZE> WHITE_PAWN_FRONTSPAN_BITBOARD =
    GetWhitePawnFrontspanBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> BLACK_PAWN_FRONTSPAN_BITBOARD =
    GetBlackPawnFrontspanBitboard();
constexpr std::array<bitboard_t, BOARD_SIDE> PAWN_NEIGHBOURS_BITBOARD = GetPawnNeighboursBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> WHITE_PAWN_BACKWARD_SENTRY_BITBOARD =
    GetWhitePawnBackwardSentryBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> BLACK_PAWN_BACKWARD_SENTRY_BITBOARD =
    GetBlackPawnBackwardSentryBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> WHITE_PAWN_PASSED_BITBOARD =
    GetWhitePawnPassedBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> BLACK_PAWN_PASSED_BITBOARD =
    GetBlackPawnPassedBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> PAWN_CONNECTED_BITBOARD = GetPawnConnectedBitboard();
constexpr std::array<uint8_t, 1 << BOARD_SIDE> PAWN_ISLANDS_COUNT = GetPawnIslandsCount();

template <bool shield, bool inverted>
constexpr std::array<ScorePair, (1 << 6)> GetShieldOrStormWeights() {
    std::array<ScorePair, (1 << 6)> res{};
    for (uint8_t i = 0; i < (1 << 6); i++) {
        for (uint8_t j = 0; j < 6; j++) {
            if (q_util::CheckBit(i, j)) {
                constexpr uint8_t INVERTED[6] = {2, 1, 0, 5, 4, 3};
                uint16_t feature = static_cast<uint16_t>(shield ? Feature::KingPawnShield1
                                                                : Feature::KingPawnStorm1);
                res[i] +=
                    MODEL_WEIGHTS[static_cast<uint16_t>(feature) + (inverted ? j : INVERTED[j])];
            }
        }
    }
    return res;
};

constexpr std::array<ScorePair, (1 << 6)> SHIELD_WEIGHTS = GetShieldOrStormWeights<true, false>();
constexpr std::array<ScorePair, (1 << 6)> SHIELD_WEIGHTS_INVERTED =
    GetShieldOrStormWeights<true, true>();
constexpr std::array<ScorePair, (1 << 6)> STORM_WEIGHTS = GetShieldOrStormWeights<false, false>();
constexpr std::array<ScorePair, (1 << 6)> STORM_WEIGHTS_INVERTED =
    GetShieldOrStormWeights<false, true>();

uint8_t GetLInftyDistance(const coord_t src, const coord_t dst) {
    return q_util::GetLInftyDistance(GetRank(src), GetFile(src), GetRank(dst), GetFile(dst));
}

template <Color c>
bool IsPawnSurelyUnstoppable(const Board& board, const coord_t pawn_coord) {
    constexpr auto COLORED_PAWN_FRONTSPAN =
        (c == Color::White ? WHITE_PAWN_FRONTSPAN_BITBOARD : BLACK_PAWN_FRONTSPAN_BITBOARD);
    if (Q_LIKELY(q_util::GetBitCount(board.bb_pieces[static_cast<uint8_t>(GetInvertedColor(c))]) >
                 1)) {
        return false;
    }
    if (COLORED_PAWN_FRONTSPAN[pawn_coord] & (~board.bb_pieces[EMPTY_CELL])) {
        return false;
    }
    if (q_util::ContainsBits(KING_ATTACK_BITBOARD[board.bb_pieces[MakeCell(c, Piece::King)]],
                             COLORED_PAWN_FRONTSPAN[pawn_coord])) {
        return true;
    }
    uint8_t pawn_dst = (c == Color::White ? BOARD_SIDE - GetRank(pawn_coord) : GetRank(pawn_coord));
    if (board.move_side != c) {
        pawn_dst++;
    }
    uint8_t king_dst = GetLInftyDistance(
        q_util::GetLowestBit(board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::King)]),
        pawn_coord);
    return pawn_dst < king_dst;
}

template bool IsPawnSurelyUnstoppable<Color::White>(const Board& board, const coord_t pawn_coord);
template bool IsPawnSurelyUnstoppable<Color::Black>(const Board& board, const coord_t pawn_coord);

}  // namespace q_eval