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

constexpr q_core::bitboard_t WHITE_KING_SHIELDED_BITBOARD =
    q_core::RANK_BITBOARD[0] | q_core::RANK_BITBOARD[1];
constexpr q_core::bitboard_t BLACK_KING_SHIELDED_BITBOARD =
    q_core::RANK_BITBOARD[q_core::BOARD_SIDE - 1] | q_core::RANK_BITBOARD[q_core::BOARD_SIDE - 2];

KingSafety GetKingSafety(const q_core::Board& board, const Color color) {
    KingSafety king_safety = KingSafety{.pawn_shield_mask = 0, .pawn_storm_mask = 0, .is_side_queenside = false};
    const coord_t king_pos = q_util::GetLowestBit(board.bb_pieces[MakeCell(color, Piece::King)]);
    const subcoord_t king_rank = GetRank(king_pos);
    const subcoord_t king_file = GetFile(king_pos);
    bool king_can_castle = color == Color::White
                               ? IsAnyCastlingAllowed(board.castling & Castling::WhiteAll)
                               : IsAnyCastlingAllowed(board.castling & Castling::BlackAll);
    bool is_king_shielded =
        (color == Color::White ? WHITE_KING_SHIELDED_BITBOARD : BLACK_KING_SHIELDED_BITBOARD) &
        MakeBitboardFromCoord(king_pos);
    if (king_can_castle || !is_king_shielded) {
        return king_safety;
    }
    const int8_t dir = (color == Color::White ? 1 : -1);
    const bitboard_t our_pawns = board.bb_pieces[MakeCell(color, Piece::Pawn)];
    const bitboard_t enemy_pawns = board.bb_pieces[MakeCell(GetInvertedColor(color), Piece::Pawn)];
    const uint8_t shield_mask1 =
        ((our_pawns & RANK_BITBOARD[king_rank + dir]) >>
         (color == Color::White ? BOARD_SIDE - 1 + king_pos : king_pos - BOARD_SIDE - 1)) &
        (BOARD_SIDE - 1);
    const uint8_t shield_mask2 =
        ((our_pawns & RANK_BITBOARD[king_rank + dir * 2]) >>
         (color == Color::White ? BOARD_SIDE * 2 - 1 + king_pos : king_pos - BOARD_SIDE * 2 - 1)) &
        (BOARD_SIDE - 1);
    const uint8_t storm_mask2 =
        ((enemy_pawns & RANK_BITBOARD[king_rank + dir * 2]) >>
         (color == Color::White ? BOARD_SIDE * 2 - 1 + king_pos : king_pos - BOARD_SIDE * 2 - 1)) &
        (BOARD_SIDE - 1);
    const uint8_t storm_mask3 =
        ((enemy_pawns & RANK_BITBOARD[king_rank + dir * 3]) >>
         (color == Color::White ? BOARD_SIDE * 3 - 1 + king_pos : king_pos - BOARD_SIDE * 3 - 1)) &
        (BOARD_SIDE - 1);
    king_safety.pawn_shield_mask = (shield_mask1 << 3 | shield_mask2);
    king_safety.pawn_storm_mask = (storm_mask2 << 3 | storm_mask3);
    king_safety.is_side_queenside = king_file < BOARD_SIDE / 2;
    return king_safety;
}

}  // namespace q_eval