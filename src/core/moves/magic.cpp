#include "magic.h"

#include <algorithm>
#include <vector>

#include "../../util/bit.h"
#include "../util.h"

namespace q_core {

template <Color c>
constexpr std::array<bitboard_t, BOARD_SIZE> GetPawnReversedAttackBitboard() {
    std::array<bitboard_t, BOARD_SIZE> res{};
    int8_t pawn_move_delta = (c == Color::White ? -BOARD_SIDE : BOARD_SIDE);
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if (IsCoordValidAndDefined(i + pawn_move_delta - 1) && GetFile(i) != 0) {
            res[i] |= (1ULL << (i + pawn_move_delta - 1));
        }
        if (IsCoordValidAndDefined(i + pawn_move_delta + 1) && GetFile(i) != BOARD_SIDE - 1) {
            res[i] |= (1ULL << (i + pawn_move_delta + 1));
        }
    }
    return res;
}

constexpr std::array<bitboard_t, BOARD_SIZE> GetKnightAttackBitboard() {
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

constexpr std::array<bitboard_t, BOARD_SIZE> GetKingAttackBitboard() {
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

constexpr std::array<bitboard_t, BOARD_SIZE> WHITE_PAWN_REVERSED_ATTACK_BITBOARD =
    GetPawnReversedAttackBitboard<Color::White>();
constexpr std::array<bitboard_t, BOARD_SIZE> BLACK_PAWN_REVERSED_ATTACK_BITBOARD =
    GetPawnReversedAttackBitboard<Color::Black>();
constexpr std::array<bitboard_t, BOARD_SIZE> KNIGHT_ATTACK_BITBOARD = GetKnightAttackBitboard();
constexpr std::array<bitboard_t, BOARD_SIZE> KING_ATTACK_BITBOARD = GetKingAttackBitboard();

constexpr uint8_t BISHOP_SHARING[64] = {0, 2, 4,  4,  4,  4,  12, 14, 0, 2, 5,  5,  5,  5,  12, 14,
                                        0, 2, 6,  6,  6,  6,  12, 14, 0, 2, 7,  7,  7,  7,  12, 14,
                                        1, 3, 8,  8,  8,  8,  13, 15, 1, 3, 9,  9,  9,  9,  13, 15,
                                        1, 3, 10, 10, 10, 10, 13, 15, 1, 3, 11, 11, 11, 11, 13, 15};

constexpr uint8_t ROOK_SHARING[64] = {
    0,  1,  2,  3,  4,  5,  6,  7,  1,  0,  3,  2,  5,  4,  7,  6,  8,  9,  10, 11, 12, 13,
    14, 15, 9,  8,  11, 10, 13, 12, 15, 14, 16, 17, 18, 19, 20, 21, 22, 23, 17, 16, 19, 18,
    21, 20, 23, 22, 24, 25, 26, 27, 28, 29, 30, 31, 25, 24, 27, 26, 29, 28, 31, 30};

template <Piece p>
void MagicBitboard::FillLookupTable(const std::array<uint64_t, 64>& piece_offset) {
    auto& piece_entry = (p == Piece::Bishop ? bishop_entry : rook_entry);
    bitboard_t* piece_lookup = (p == Piece::Bishop ? bishop_lookup : rook_lookup);
    std::array<int8_t, 4> dx = (p == Piece::Bishop ? std::array<int8_t, 4>({-1, -1, 1, 1})
                                                   : std::array<int8_t, 4>({0, 0, -1, 1}));
    std::array<int8_t, 4> dy = (p == Piece::Bishop ? std::array<int8_t, 4>({-1, 1, 1, -1})
                                                   : std::array<int8_t, 4>({-1, 1, 0, 0}));
    for (coord_t i = 0; i < 64; i++) {
        uint64_t submask_size = (1ULL << q_util::GetBitCount(piece_entry[i].mask));
        for (uint64_t submask = 0; submask < submask_size; submask++) {
            const bitboard_t occupied = q_util::DepositBits(submask, piece_entry[i].mask);
            bitboard_t& res = piece_lookup[piece_offset[i] + submask];
            for (uint8_t dir = 0; dir < 4; dir++) {
                subcoord_t x = GetRank(i);
                subcoord_t y = GetFile(i);
                for (;;) {
                    res |= MakeBitboardFromCoord(MakeCoord(x, y));
                    x += dx[dir];
                    y += dy[dir];
                    if (!(IsSubcoordValid(x) && IsSubcoordValid(y))) {
                        break;
                    }
                    if (q_util::CheckBit(occupied, MakeCoord(x, y))) {
                        res |= MakeBitboardFromCoord(MakeCoord(x, y));
                        break;
                    }
                }
                q_util::ClearBit(res, i);
            }
        }
    }
}

MagicBitboard::MagicBitboard() {
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        const subcoord_t x = GetRank(i);
        const subcoord_t y = GetFile(i);
        bishop_entry[i].mask =
            (LEFT_DIAGONAL_BITBOARD[x + y] ^ RIGHT_DIAGONAL_BITBOARD[x - y + BOARD_SIDE - 1]) &
            (~FRAME_BITBOARD);
        bishop_entry[i].postmask =
            LEFT_DIAGONAL_BITBOARD[x + y] ^ RIGHT_DIAGONAL_BITBOARD[x - y + BOARD_SIDE - 1];
        rook_entry[i].mask = (RANK_BITBOARD[x] ^ FILE_BITBOARD[y]);
        if (GetRank(i) != 0) {
            rook_entry[i].mask &= (~RANK_BITBOARD[0]);
        }
        if (GetFile(i) != 0) {
            rook_entry[i].mask &= (~FILE_BITBOARD[0]);
        }
        if (GetRank(i) != BOARD_SIDE - 1) {
            rook_entry[i].mask &= (~RANK_BITBOARD[BOARD_SIDE - 1]);
        }
        if (GetFile(i) != BOARD_SIDE - 1) {
            rook_entry[i].mask &= (~FILE_BITBOARD[BOARD_SIDE - 1]);
        }
        rook_entry[i].postmask = RANK_BITBOARD[x] ^ FILE_BITBOARD[y];
    }
    std::array<uint64_t, 64> bishop_offset;
    std::array<uint64_t, 64> rook_offset;
    std::array<std::vector<uint8_t>, 16> bishop_groups;
    std::array<std::vector<uint8_t>, 32> rook_groups;
    for (uint8_t i = 0; i < 64; i++) {
        bishop_groups[BISHOP_SHARING[i]].push_back(i);
        rook_groups[ROOK_SHARING[i]].push_back(i);
    }
    uint64_t count = 0;
    for (size_t i = 0; i < bishop_groups.size(); i++) {
        Q_ASSERT(bishop_groups[i].size() == 4);
        const uint64_t max_len =
            std::max({q_util::GetBitCount(bishop_entry[bishop_groups[i][0]].mask),
                      q_util::GetBitCount(bishop_entry[bishop_groups[i][1]].mask),
                      q_util::GetBitCount(bishop_entry[bishop_groups[i][2]].mask),
                      q_util::GetBitCount(bishop_entry[bishop_groups[i][3]].mask)});
        for (const auto& j : bishop_groups[i]) {
            bishop_offset[j] = count;
            bishop_entry[j].lookup = bishop_lookup + count;
        }
        count += (1ULL << max_len);
    }
    count = 0;
    for (size_t i = 0; i < rook_groups.size(); i++) {
        Q_ASSERT(rook_groups[i].size() == 2);
        const uint64_t max_len =
            std::max({q_util::GetBitCount(rook_entry[rook_groups[i][0]].mask),
                      q_util::GetBitCount(rook_entry[rook_groups[i][1]].mask)});
        for (const auto& j : rook_groups[i]) {
            rook_offset[j] = count;
            rook_entry[j].lookup = rook_lookup + count;
        }
        count += (1ULL << max_len);
    }
    FillLookupTable<Piece::Bishop>(bishop_offset);
    FillLookupTable<Piece::Rook>(rook_offset);
}

const MagicBitboard MAGIC_BITBOARD;

bitboard_t GetBishopAttackBitboard(const bitboard_t occupied, coord_t src) {
    const auto& entry = MAGIC_BITBOARD.bishop_entry[src];
    return entry.lookup[q_util::ExtractBits(occupied, entry.mask)] & entry.postmask;
}

bitboard_t GetRookAttackBitboard(const bitboard_t occupied, coord_t src) {
    const auto& entry = MAGIC_BITBOARD.rook_entry[src];
    return entry.lookup[q_util::ExtractBits(occupied, entry.mask)] & entry.postmask;
}

}  // namespace q_core