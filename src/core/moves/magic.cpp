#include "magic.h"

#include <algorithm>
#include <vector>

#include "../../util/bit.h"
#include "core/board/geometry.h"
#include "core/board/types.h"

namespace q_core {

#ifdef NO_AVX2
static constexpr std::array<uint64_t, BOARD_SIZE> ROOK_MAGIC_CONSTS = {
    0x0080002040001882, 0x08c0009000200241, 0x0080081004816000, 0x0080048010008800,
    0x0200102004020148, 0x2100024400090008, 0x0080020020804500, 0x01800840e0800100,
    0x080d800188204007, 0x0280804000802000, 0x8000802004841000, 0xa400801000801804,
    0x2420802800800400, 0x0420800200800400, 0x0001000100048200, 0x8000800100004080,
    0x4202208000804002, 0x1020044010002441, 0x1000848020021000, 0x0402848008003000,
    0xc002808028000400, 0x0201010042040008, 0x008804000108100a, 0x0000420004a10c44,
    0x200240008000b080, 0x1000200040100244, 0x0002844200160020, 0x40100021000a1102,
    0x0480040080080080, 0x6210040080060080, 0x002a000200015c18, 0x0810404200040483,
    0x200040102d800080, 0x2000401000402002, 0x20460010820042a0, 0x2010810801801000,
    0x0018040080801800, 0x0008802a00800401, 0x0100189014000502, 0x1000a40082002441,
    0x0180400088288000, 0x10020081004a0024, 0x0009002004110040, 0x2001026110010008,
    0x1008000401808008, 0x2001000804010006, 0x4054c20004010100, 0x20a0004884020005,
    0x0124402210810200, 0x0040004860088080, 0x0016100220028a80, 0x000b080181100080,
    0x8101008c10080100, 0x1a21040002008080, 0x0040508a08410400, 0x90088181014c0600,
    0x0010402104908001, 0x0000228040120102, 0x800020001102a943, 0x4000e10088100085,
    0x4c82880010030103, 0x040b008604001841, 0x0001910802500094, 0x0840022248840302};

static constexpr std::array<uint64_t, BOARD_SIZE> BISHOP_MAGIC_CONSTS{
    0x0004480a48c20202, 0x001c900a0261020a, 0x08a8248402802164, 0x4004141182130034,
    0x0004242000880a06, 0x3102902420000801, 0x4005180390081120, 0x1012004301882020,
    0x0250c0cc28008120, 0x2000200802a10440, 0x0200080604002820, 0x24000404008102c0,
    0x1000440421022001, 0x000190c820101000, 0x2040010108824000, 0x0080012104022000,
    0x0028324009080080, 0x000400600c208220, 0x2010003800801050, 0x051800c401405200,
    0x180c011080a08100, 0x0002000500424204, 0x440a0c10808420c1, 0x1082000101098282,
    0x2021056010141800, 0x0030085074010c00, 0x0028020104040011, 0x8011480000820040,
    0x00010044a1004000, 0x3418108008104c00, 0x0820822118821010, 0x1005020140221104,
    0x0102101008400600, 0x0201210840101000, 0x2020282800900080, 0x11000c0100100900,
    0x0020048400288020, 0x0820042020190088, 0x4848080450008202, 0x0380940c80914201,
    0x20080a0874002080, 0x0054010308001028, 0x8140911090000810, 0x0080090148001900,
    0x4404c01009020082, 0x4040082680a01100, 0x5002241902029400, 0x0004044040c80200,
    0x02042a0a10240000, 0x0200218804900000, 0x041440440c042084, 0x8e0800002a080010,
    0x4600b43082022110, 0x0008201430208100, 0x0030020848048100, 0x8048010144050046,
    0x5002008404010442, 0x100430c228842001, 0x1002400a09098800, 0x4100498000c20218,
    0x8c08001440ad0300, 0x0100240802089200, 0x000040300a820443, 0x1020289f04408200};

static constexpr std::array<uint64_t, BOARD_SIZE> ROOK_SHIFT_CONSTS = {
    52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54,
    54, 53, 53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54,
    54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52};

static constexpr std::array<uint64_t, BOARD_SIZE> BISHOP_SHIFT_CONSTS = {
    58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 57, 57, 57, 57,
    59, 59, 59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57,
    57, 57, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58};
#endif

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
#ifndef NO_AVX2
            bitboard_t& res = piece_lookup[piece_offset[i] + submask];
#else
            const auto& magics = (p == Piece::Rook) ? ROOK_MAGIC_CONSTS : BISHOP_MAGIC_CONSTS;
            const auto& shifts = (p == Piece::Rook) ? ROOK_SHIFT_CONSTS : BISHOP_SHIFT_CONSTS;
            const auto pos = static_cast<size_t>((occupied * magics[i]) >> shifts[i]);
            bitboard_t& res = piece_lookup[piece_offset[i] + pos];
#endif
            for (uint8_t dir = 0; dir < 4; dir++) {
                subcoord_t rank = GetRank(i);
                subcoord_t file = GetFile(i);
                for (;;) {
                    res |= MakeBitboardFromCoord(MakeCoord(rank, file));
                    rank += dx[dir];
                    file += dy[dir];
                    if (!(IsSubcoordValid(rank) && IsSubcoordValid(file))) {
                        break;
                    }
                    if (q_util::CheckBit(occupied, MakeCoord(rank, file))) {
                        res |= MakeBitboardFromCoord(MakeCoord(rank, file));
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
            q_util::ClearBits(rook_entry[i].mask, RANK_BITBOARD[0]);
        }
        if (GetFile(i) != 0) {
            q_util::ClearBits(rook_entry[i].mask, FILE_BITBOARD[0]);
        }
        if (GetRank(i) != BOARD_SIDE - 1) {
            q_util::ClearBits(rook_entry[i].mask, RANK_BITBOARD[BOARD_SIDE - 1]);
        }
        if (GetFile(i) != BOARD_SIDE - 1) {
            q_util::ClearBits(rook_entry[i].mask, FILE_BITBOARD[BOARD_SIDE - 1]);
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

#ifndef NO_AVX2
bitboard_t GetBishopAttackBitboard(const bitboard_t occupied, coord_t src) {
    const auto& entry = MAGIC_BITBOARD.bishop_entry[src];
    return entry.lookup[q_util::ExtractBits(occupied, entry.mask)] & entry.postmask;
}

bitboard_t GetRookAttackBitboard(const bitboard_t occupied, coord_t src) {
    const auto& entry = MAGIC_BITBOARD.rook_entry[src];
    return entry.lookup[q_util::ExtractBits(occupied, entry.mask)] & entry.postmask;
}
#else
bitboard_t GetBishopAttackBitboard(const bitboard_t occupied, coord_t src) {
    const auto& entry = MAGIC_BITBOARD.bishop_entry[src];
    const size_t index = static_cast<size_t>(((occupied & entry.mask) * BISHOP_MAGIC_CONSTS[src]) >>
                                             BISHOP_SHIFT_CONSTS[src]);
    return entry.lookup[index] & entry.postmask;
}

bitboard_t GetRookAttackBitboard(const bitboard_t occupied, coord_t src) {
    const auto& entry = MAGIC_BITBOARD.rook_entry[src];
    const size_t index = static_cast<size_t>(((occupied & entry.mask) * ROOK_MAGIC_CONSTS[src]) >>
                                             ROOK_SHIFT_CONSTS[src]);
    return entry.lookup[index] & entry.postmask;
}
#endif

}  // namespace q_core