#include "eval_features.h"

#include <array>

#include "../core/board/geometry.h"
#include "../core/moves/magic.h"
#include "../core/util.h"
#include "../util/math.h"
#include "model.h"

namespace q_eval {

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetWhitePawnFrontspanBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        for (q_core::subcoord_t j = rank + 1; j < q_core::BOARD_SIDE; j++) {
            res[i] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file));
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetBlackPawnFrontspanBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        for (q_core::subcoord_t j = rank - 1; j >= 0; j--) {
            res[i] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file));
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIDE> GetPawnNeighboursBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIDE> res{};
    for (q_core::coord_t file = 0; file < q_core::BOARD_SIDE; file++) {
        if (file > 0) {
            res[file] |= q_core::FILE_BITBOARD[file - 1];
        }
        if (file < q_core::BOARD_SIDE - 1) {
            res[file] |= q_core::FILE_BITBOARD[file + 1];
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetWhitePawnBackwardSentryBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = q_core::MakeCoord(1, 0);
         i < q_core::MakeCoord(q_core::BOARD_SIDE - 2, 0); i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        if (file > 0) {
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(rank + 2, file - 1));
        }
        if (file < q_core::BOARD_SIDE - 1) {
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(rank + 2, file + 1));
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetBlackPawnBackwardSentryBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = q_core::MakeCoord(2, 0);
         i < q_core::MakeCoord(q_core::BOARD_SIDE - 1, 0); i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        if (file > 0) {
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(rank - 2, file - 1));
        }
        if (file < q_core::BOARD_SIDE - 1) {
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(rank - 2, file + 1));
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetWhitePawnPassedBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        for (q_core::subcoord_t j = rank + 1; j < q_core::BOARD_SIDE; j++) {
            if (file > 0) {
                res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file - 1));
            }
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file));
            if (file < q_core::BOARD_SIDE - 1) {
                res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file + 1));
            }
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetBlackPawnPassedBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        for (q_core::subcoord_t j = rank - 1; j >= 0; j--) {
            if (file > 0) {
                res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file - 1));
            }
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file));
            if (file < q_core::BOARD_SIDE - 1) {
                res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(j, file + 1));
            }
        }
    }
    return res;
}

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> GetPawnConnectedBitboard() {
    std::array<q_core::bitboard_t, q_core::BOARD_SIZE> res{};
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        q_core::subcoord_t rank = q_core::GetRank(i);
        q_core::subcoord_t file = q_core::GetFile(i);
        if (file > 0) {
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(rank, file - 1));
        }
        if (file < q_core::BOARD_SIDE - 1) {
            res[file] |= q_core::MakeBitboardFromCoord(q_core::MakeCoord(rank, file + 1));
        }
    }
    return res;
}

constexpr std::array<uint8_t, 1 << q_core::BOARD_SIDE> GetPawnIslandsCount() {
    std::array<uint8_t, 1 << q_core::BOARD_SIDE> res{};
    for (uint16_t mask = 0; mask < (1 << q_core::BOARD_SIDE); mask++) {
        uint8_t ans = 0;
        bool prev = false;
        for (uint8_t file = 0; file < q_core::BOARD_SIDE; file++) {
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

constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> WHITE_PAWN_FRONTSPAN_BITBOARD =
    GetWhitePawnFrontspanBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> BLACK_PAWN_FRONTSPAN_BITBOARD =
    GetBlackPawnFrontspanBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIDE> PAWN_NEIGHBOURS_BITBOARD =
    GetPawnNeighboursBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE>
    WHITE_PAWN_BACKWARD_SENTRY_BITBOARD = GetWhitePawnBackwardSentryBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE>
    BLACK_PAWN_BACKWARD_SENTRY_BITBOARD = GetBlackPawnBackwardSentryBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> WHITE_PAWN_PASSED_BITBOARD =
    GetWhitePawnPassedBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> BLACK_PAWN_PASSED_BITBOARD =
    GetWhitePawnPassedBitboard();
constexpr std::array<q_core::bitboard_t, q_core::BOARD_SIZE> PAWN_CONNECTED_BITBOARD =
    GetPawnConnectedBitboard();
constexpr std::array<uint8_t, 1 << q_core::BOARD_SIDE> PAWN_ISLANDS_COUNT =
    GetPawnIslandsCount();

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

constexpr std::array<ScorePair, (1 << 6)> SHIELD_WEIGHTS =
    GetShieldOrStormWeights<true, false>();
constexpr std::array<ScorePair, (1 << 6)> SHIELD_WEIGHTS_INVERTED =
    GetShieldOrStormWeights<true, true>();
constexpr std::array<ScorePair, (1 << 6)> STORM_WEIGHTS =
    GetShieldOrStormWeights<false, false>();
constexpr std::array<ScorePair, (1 << 6)> STORM_WEIGHTS_INVERTED =
    GetShieldOrStormWeights<false, true>();

uint8_t GetLInftyDistance(const q_core::coord_t src, const q_core::coord_t dst) {
    return q_util::GetLInftyDistance(q_core::GetRank(src), q_core::GetFile(src),
                                     q_core::GetRank(dst), q_core::GetFile(dst));
}

template <q_core::Color c>
bool IsPawnSurelyUnstoppable(const q_core::Board& board, const q_core::coord_t pawn_coord) {
    constexpr auto COLORED_PAWN_FRONTSPAN =
        (c == q_core::Color::White ? WHITE_PAWN_FRONTSPAN_BITBOARD : BLACK_PAWN_FRONTSPAN_BITBOARD);
    if (Q_LIKELY(q_util::GetBitCount(
                     board.bb_pieces[static_cast<uint8_t>(q_core::GetInvertedColor(c))]) > 1)) {
        return false;
    }
    if (COLORED_PAWN_FRONTSPAN[pawn_coord] & (~board.bb_pieces[q_core::EMPTY_CELL])) {
        return false;
    }
    if (q_util::ContainsBits(
            q_core::KING_ATTACK_BITBOARD[board.bb_pieces[q_core::MakeCell(c, q_core::Piece::King)]],
            COLORED_PAWN_FRONTSPAN[pawn_coord])) {
        return true;
    }
    uint8_t pawn_dst = (c == q_core::Color::White ? q_core::BOARD_SIDE - q_core::GetRank(pawn_coord)
                                                  : q_core::GetRank(pawn_coord));
    if (board.move_side != c) {
        pawn_dst++;
    }
    uint8_t king_dst = GetLInftyDistance(
        q_util::GetLowestBit(
            board.bb_pieces[q_core::MakeCell(q_core::GetInvertedColor(c), q_core::Piece::King)]),
        pawn_coord);
    return pawn_dst < king_dst;
}

template bool IsPawnSurelyUnstoppable<q_core::Color::White>(const q_core::Board& board,
                                                            const q_core::coord_t pawn_coord);
template bool IsPawnSurelyUnstoppable<q_core::Color::Black>(const q_core::Board& board,
                                                            const q_core::coord_t pawn_coord);

}  // namespace q_eval