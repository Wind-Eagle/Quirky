#include "evaluator.h"

#include "../core/moves/magic.h"
#include "../core/util.h"
#include "eval_features.h"
#include "pawns.h"
#include "psq.h"

using namespace q_core;

namespace q_eval {

template <EvaluationType type, Color c>
void AddFeature(typename EvaluationResultType<type>::type& score, const Feature feature,
                const int8_t count) {
    /*Q_ASSERT(MODEL_WEIGHTS[static_cast<uint16_t>(feature)].GetFirst() != 0 ||
             MODEL_WEIGHTS[static_cast<uint16_t>(feature)].GetSecond() != 0);*/
    if constexpr (type == EvaluationType::Value) {
        if constexpr (c == Color::White) {
            score += MODEL_WEIGHTS[static_cast<uint16_t>(feature)] * count;
        } else {
            score -= MODEL_WEIGHTS[static_cast<uint16_t>(feature)] * count;
        }
    } else {
        if constexpr (c == Color::White) {
            score[static_cast<uint16_t>(feature)] += count;
        } else {
            score[static_cast<uint16_t>(feature)] -= count;
        }
    }
}

template <EvaluationType type, Color c>
void EvaluatePawns(const Board& board, typename EvaluationResultType<type>::type& score,
                   uint8_t& open_files_mask) {
    const bitboard_t our_pawns = board.bb_pieces[MakeCell(c, Piece::Pawn)];
    const bitboard_t enemy_pawns = board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)];

    const auto colored_pawn_frontspan =
        (c == Color::White ? WHITE_PAWN_FRONTSPAN_BITBOARD : BLACK_PAWN_FRONTSPAN_BITBOARD);

    uint8_t pawn_islands_mask = 0;
    bitboard_t pawn_bitboard = board.bb_pieces[MakeCell(c, Piece::Pawn)];
    while (pawn_bitboard) {
        const coord_t pawn_coord = q_util::ExtractLowestBit(pawn_bitboard);
        const subcoord_t pawn_rank = GetRank(pawn_coord);
        const subcoord_t pawn_file = GetFile(pawn_coord);
        q_util::SetBit(pawn_islands_mask, pawn_file);
        const subcoord_t relative_pawn_rank =
            (c == Color::White ? pawn_rank : InvertSubcoord(pawn_rank));
        Q_ASSUME(relative_pawn_rank > 0 && relative_pawn_rank < BOARD_SIDE - 1);

        bool is_pawn_isolated = false;
        bool is_pawn_passed = false;

        if ((c == Color::White ? WHITE_PAWN_REVERSED_ATTACK_BITBOARD[pawn_coord]
                               : BLACK_PAWN_REVERSED_ATTACK_BITBOARD[pawn_coord]) &
            our_pawns) {
            AddFeature<type, c>(score, Feature::DefendedPawn, 1);
            if (relative_pawn_rank >= DEFENDED_PAWN_FEATURE_FIRST &&
                relative_pawn_rank <= DEFENDED_PAWN_FEATURE_LAST) {
                AddFeature<type, c>(
                    score,
                    static_cast<Feature>(static_cast<uint16_t>(Feature::DefendedPawnOnRank4) +
                                         relative_pawn_rank - DEFENDED_PAWN_FEATURE_FIRST),
                    1);
            }
        }

        if (!(PAWN_NEIGHBOURS_BITBOARD[pawn_file] & our_pawns)) {
            is_pawn_isolated = true;
            AddFeature<type, c>(score, Feature::IsolatedPawn, 1);
        }

        if (!(colored_pawn_frontspan[pawn_coord] & (our_pawns | enemy_pawns))) {
            AddFeature<type, c>(score, Feature::OpenedPawn, 1);

            if (!((c == Color::White ? WHITE_PAWN_PASSED_BITBOARD[pawn_coord]
                                     : BLACK_PAWN_PASSED_BITBOARD[pawn_coord]) &
                  enemy_pawns)) {
                is_pawn_passed = true;
                AddFeature<type, c>(score, Feature::PassedPawn, 1);
                if (relative_pawn_rank >= PASSED_PAWN_FEATURE_FIRST &&
                    relative_pawn_rank <= PASSED_PAWN_FEATURE_LAST) {
                    AddFeature<type, c>(
                        score,
                        static_cast<Feature>(static_cast<uint16_t>(Feature::PassedPawnOnRank4) +
                                             relative_pawn_rank - PASSED_PAWN_FEATURE_FIRST),
                        1);
                }
                if (IsPawnSurelyUnstoppable<c>(board, pawn_coord)) {
                    AddFeature<type, c>(score, Feature::SurelyUnstoppablePawn, 1);
                }
            }
        }

        if (colored_pawn_frontspan[pawn_coord] & our_pawns) {
            AddFeature<type, c>(score, Feature::DoubledPawn, 1);
            if (is_pawn_isolated) {
                AddFeature<type, c>(score, Feature::DoubledIsolatedPawn, 1);
            }
            if (is_pawn_passed) {
                AddFeature<type, c>(score, Feature::DoubledPassedPawn, 1);
            }
        }

        if (!((c == Color::White ? (~(BLACK_PAWN_PASSED_BITBOARD[pawn_coord]))
                                 : (~(WHITE_PAWN_PASSED_BITBOARD[pawn_coord]))) |
              PAWN_CONNECTED_BITBOARD[pawn_coord] & (~FILE_BITBOARD[pawn_rank]) & our_pawns) &&
            ((c == Color::White ? WHITE_PAWN_BACKWARD_SENTRY_BITBOARD[pawn_coord]
                                : BLACK_PAWN_BACKWARD_SENTRY_BITBOARD[pawn_coord]) &
             enemy_pawns)) {
            AddFeature<type, c>(score, Feature::BackwardPawn, 1);
        }

        if (PAWN_CONNECTED_BITBOARD[pawn_coord] & our_pawns) {
            AddFeature<type, c>(score, Feature::ConnectedPawn, 1);
            if (relative_pawn_rank >= CONNECTED_PAWN_FEATURE_FIRST &&
                relative_pawn_rank <= CONNECTED_PAWN_FEATURE_LAST) {
                AddFeature<type, c>(
                    score,
                    static_cast<Feature>(static_cast<uint16_t>(Feature::ConnectedPawnOnRank4) +
                                         relative_pawn_rank - CONNECTED_PAWN_FEATURE_FIRST),
                    1);
            }
        }
    }
    AddFeature<type, c>(score, Feature::PawnIslands, PAWN_ISLANDS_COUNT[pawn_islands_mask]);
    if (!our_pawns) {
        AddFeature<type, c>(score, Feature::NoPawns, 1);
    }
    open_files_mask = (~pawn_islands_mask);
}

template <EvaluationType type>
PawnHashTableEntry EvaluatePawns(const Board& board,
                                 typename EvaluationResultType<type>::type& res) {
    uint8_t white_open_files_mask = 0;
    uint8_t black_open_files_mask = 0;
    typename EvaluationResultType<type>::type score{};
    EvaluatePawns<type, Color::White>(board, score, white_open_files_mask);
    EvaluatePawns<type, Color::Black>(board, score, black_open_files_mask);
    if constexpr (type == EvaluationType::Vector) {
        return PawnHashTableEntry{ScorePair(), white_open_files_mask, black_open_files_mask};
    } else {
        return PawnHashTableEntry{score, white_open_files_mask, black_open_files_mask};
    }
}

template <EvaluationType type, Color c>
void EvaluateKNBRQ(const Board& board, typename EvaluationResultType<type>::type& score,
                   const PawnHashTableEntry pawn_hash_table_entry) {
    bitboard_t open_files = pawn_hash_table_entry.white_open_files_mask &
                            pawn_hash_table_entry.black_open_files_mask;
    bitboard_t half_open_files =
        (c == Color::White ? pawn_hash_table_entry.white_open_files_mask
                           : pawn_hash_table_entry.black_open_files_mask);
    if (board.bb_pieces[MakeCell(c, Piece::Knight)]) {
        AddFeature<type, c>(score, Feature::KnightPair,
                            q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Knight)]) - 1);
    }
    if (board.bb_pieces[MakeCell(c, Piece::Bishop)]) {
        AddFeature<type, c>(score, Feature::BishopPair,
                            q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Bishop)]) - 1);
    }
    AddFeature<type, c>(score, Feature::RookOnOpenFile,
                        q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Rook)] &
                                            q_util::ScatterByte(open_files)));
    AddFeature<type, c>(score, Feature::RookOnHalfOpenFile,
                        q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Rook)] &
                                            q_util::ScatterByte(half_open_files)));
    const coord_t enemy_king_pos =
        q_util::GetLowestBit(board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::King)]);
    if (board.bb_pieces[MakeCell(c, Piece::Queen)]) {
        const coord_t queen_pos = q_util::GetLowestBit(board.bb_pieces[MakeCell(c, Piece::Queen)]);
        const uint8_t queen_king_distance = GetLInftyDistance(enemy_king_pos, queen_pos);
        Q_ASSUME(queen_king_distance > 0);
        if (queen_king_distance >= QUEEN_KING_DISTANCE_FEATURE_FIRST &&
            queen_king_distance <= QUEEN_KING_DISTANCE_FEATURE_LAST) {
            AddFeature<type, c>(
                score,
                static_cast<Feature>(static_cast<uint16_t>(Feature::QueenKingDistance1) +
                                     queen_king_distance - QUEEN_KING_DISTANCE_FEATURE_FIRST),
                1);
        }
    }
    const coord_t king_pos = q_util::GetLowestBit(board.bb_pieces[MakeCell(c, Piece::King)]);
    const subcoord_t king_rank = GetRank(king_pos);
    const subcoord_t king_file = GetFile(king_pos);
    bool is_king_castled = c == Color::White ? IsAnyCastlingAllowed(board.castling & Castling::WhiteAll)
                            : IsAnyCastlingAllowed(board.castling & Castling::BlackAll);
    bool is_king_shielded = (c == Color::White ? WHITE_KING_SHIELDED_BITBOARD : BLACK_KING_SHIELDED_BITBOARD) &
            MakeBitboardFromCoord(king_pos);
    if (!is_king_castled && is_king_shielded) {
        constexpr int8_t DIR = (c == Color::White ? 1 : -1);
        const bitboard_t our_pawns = board.bb_pieces[MakeCell(c, Piece::Pawn)];
        const bitboard_t enemy_pawns = board.bb_pieces[MakeCell(GetInvertedColor(c), Piece::Pawn)];
        const uint8_t shield_mask1 =
            ((our_pawns & RANK_BITBOARD[king_rank + DIR]) >>
             (c == Color::White ? BOARD_SIDE - 1 + king_pos : king_pos - BOARD_SIDE - 1)) &
            7;
        const uint8_t shield_mask2 =
            ((our_pawns & RANK_BITBOARD[king_rank + DIR * 2]) >>
             (c == Color::White ? BOARD_SIDE * 2 - 1 + king_pos : king_pos - BOARD_SIDE * 2 - 1)) &
            7;
        const uint8_t storm_mask2 =
            ((enemy_pawns & RANK_BITBOARD[king_rank + DIR * 2]) >>
             (c == Color::White ? BOARD_SIDE * 2 - 1 + king_pos : king_pos - BOARD_SIDE * 2 - 1)) &
            7;
        const uint8_t storm_mask3 =
            ((enemy_pawns & RANK_BITBOARD[king_rank + DIR * 3]) >>
             (c == Color::White ? BOARD_SIDE * 3 - 1 + king_pos : king_pos - BOARD_SIDE * 3 - 1)) &
            7;
        const bool inverted = king_file >= BOARD_SIDE / 2;
        if constexpr (type == EvaluationType::Value) {
            const auto shield_weights = (inverted ? SHIELD_WEIGHTS_INVERTED : SHIELD_WEIGHTS);
            const auto storm_weights = (inverted ? STORM_WEIGHTS_INVERTED : STORM_WEIGHTS);
            score += shield_weights[shield_mask1 << 3 | shield_mask2];
            score += storm_weights[storm_mask2 << 3 | storm_mask3];
        } else {
            uint16_t magic_value =
                shield_mask1 | (shield_mask2 << 3) | (storm_mask2 << 6) | (storm_mask3 << 9);
            for (uint16_t i = 0; i < 6; i++) {
                if (q_util::CheckBit(magic_value, i)) {
                    AddFeature<type, c>(
                        score,
                        static_cast<Feature>(static_cast<uint16_t>(Feature::KingPawnShield1) + i),
                        1);
                }
            }
            for (uint16_t i = 0; i < 6; i++) {
                if (q_util::CheckBit(magic_value, i + 6)) {
                    AddFeature<type, c>(
                        score,
                        static_cast<Feature>(static_cast<uint16_t>(Feature::KingPawnStorm1) + i),
                        1);
                }
            }
        }
    }
}

template <EvaluationType type>
void EvaluateKNBRQ(const Board& board, typename EvaluationResultType<type>::type& score,
                   const PawnHashTableEntry pawn_hash_table_entry) {
    EvaluateKNBRQ<type, Color::White>(board, score, pawn_hash_table_entry);
    EvaluateKNBRQ<type, Color::Black>(board, score, pawn_hash_table_entry);
}

template <EvaluationType type>
typename EvaluationResultType<type>::type Evaluator<type>::Evaluate(const Board& board) const {
    Q_ASSERT(board.IsValid());
    Q_ASSERT([&]() {
        Tag cur_tag;
        cur_tag.BuildTag(board);
        return cur_tag == tag_;
    }());
    typename EvaluationResultType<type>::type res = tag_.GetScore();
    const auto pawn_entry = EvaluatePawns<type>(board, res);
    EvaluateKNBRQ<type>(board, res, pawn_entry);
    if constexpr (type == EvaluationType::Value) {
        if (board.move_side == Color::Black) {
            res *= -1;
        }
    }
    return res;
}

template <EvaluationType type>
score_t Evaluator<type>::GetEvaluationScore(
    const typename EvaluationResultType<type>::type score) const {
    ScorePair ans = 0;
    if constexpr (type == EvaluationType::Vector) {
        const auto& features = score.GetFeatures();
        for (size_t i = 0; i < FEATURE_COUNT; i++) {
            ans += MODEL_WEIGHTS[i];
        }
        for (size_t i = 0; i < PSQ_SIZE; i++) {
            ans += PSQ[i / BOARD_SIZE][i % BOARD_SIZE];
        }
    } else {
        ans = score;
    }
    stage_t stage = std::max(tag_.GetStage(), Tag::STAGE_MAX);
    return (ans.GetFirst() * stage + ans.GetSecond() * (Tag::STAGE_MAX - stage)) / Tag::STAGE_MAX;
}

template <EvaluationType type>
void Evaluator<type>::Tag::BuildTag(const Board& board) {
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if constexpr (type == EvaluationType::Value) {
            if (board.cells[i] != EMPTY_CELL) {
                if (GetCellColor(board.cells[i]) == Color::White) {
                    score_ += PSQ[board.cells[i]][i];
                } else {
                    score_ -= PSQ[board.cells[i]][i];
                }
                stage_ += CELL_STAGE_EVAL[board.cells[i]];
            }
        } else {
            const uint16_t index = GetPSQIndex(board.cells[i], i);
            if (board.cells[i] != EMPTY_CELL) {
                if (GetCellColor(board.cells[i]) == Color::White) {
                    score_[FEATURE_COUNT + index]++;
                } else {
                    score_[FEATURE_COUNT + index]--;
                }
                stage_ += CELL_STAGE_EVAL[board.cells[i]];
            }
        }
    }
}

constexpr std::array<ScorePair, 2> KINGSIGE_CASTLING_PSQ_UPDATE = {
    PSQ[MakeCell(Color::White, Piece::King)][WHITE_KING_INITIAL_POSITION + 2] +
        PSQ[MakeCell(Color::White, Piece::Rook)][WHITE_KING_INITIAL_POSITION + 1] -
        PSQ[MakeCell(Color::White, Piece::King)][WHITE_KING_INITIAL_POSITION] -
        PSQ[MakeCell(Color::White, Piece::Rook)][WHITE_KING_INITIAL_POSITION + 3],
    PSQ[MakeCell(Color::White, Piece::King)][BLACK_KING_INITIAL_POSITION + 2] +
        PSQ[MakeCell(Color::White, Piece::Rook)][BLACK_KING_INITIAL_POSITION + 1] -
        PSQ[MakeCell(Color::White, Piece::King)][BLACK_KING_INITIAL_POSITION] -
        PSQ[MakeCell(Color::White, Piece::Rook)][BLACK_KING_INITIAL_POSITION + 3]};

constexpr std::array<ScorePair, 2> QUEENSIGE_CASTLING_PSQ_UPDATE = {
    PSQ[MakeCell(Color::White, Piece::King)][WHITE_KING_INITIAL_POSITION - 2] +
        PSQ[MakeCell(Color::White, Piece::Rook)][WHITE_KING_INITIAL_POSITION - 1] -
        PSQ[MakeCell(Color::White, Piece::King)][WHITE_KING_INITIAL_POSITION] -
        PSQ[MakeCell(Color::White, Piece::Rook)][WHITE_KING_INITIAL_POSITION - 4],
    PSQ[MakeCell(Color::White, Piece::King)][BLACK_KING_INITIAL_POSITION - 2] +
        PSQ[MakeCell(Color::White, Piece::Rook)][BLACK_KING_INITIAL_POSITION - 1] -
        PSQ[MakeCell(Color::White, Piece::King)][BLACK_KING_INITIAL_POSITION] -
        PSQ[MakeCell(Color::White, Piece::Rook)][BLACK_KING_INITIAL_POSITION - 4]};

template <EvaluationType type>
typename Evaluator<type>::Tag Evaluator<type>::Tag::UpdateTag(const Board& board, const Move move) {
    Q_ASSERT(!IsMoveNull(move) && !IsMoveUndefined(move));
    if constexpr (type == EvaluationType::Value) {
        typename Evaluator<type>::Tag new_tag = (*this);
        const cell_t src_cell = board.cells[move.src];
        const cell_t dst_cell = board.cells[move.dst];
        if (IsMoveCastling(move)) {
            if (move.type == KINGSIDE_CASTLING_MOVE_TYPE) {
                new_tag.score_ +=
                    KINGSIGE_CASTLING_PSQ_UPDATE[static_cast<uint8_t>(board.move_side)];
            } else {
                new_tag.score_ +=
                    QUEENSIGE_CASTLING_PSQ_UPDATE[static_cast<uint8_t>(board.move_side)];
            }
            return new_tag;
        }
        new_tag.score_ -= PSQ[src_cell][move.src] + PSQ[dst_cell][move.dst];
        new_tag.stage_ -= CELL_STAGE_EVAL[dst_cell];
        if (IsMovePromotion(move)) {
            cell_t promotion_cell = MakeCell(board.move_side, GetPromotionPiece(move));
            new_tag.score_ += PSQ[promotion_cell][move.dst];
            new_tag.stage_ += CELL_STAGE_EVAL[promotion_cell];
            return new_tag;
        }
        new_tag.score_ += PSQ[src_cell][move.dst];
        if (IsMoveEnPassant(move)) {
            const coord_t taken_coord =
                (board.move_side == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
            const cell_t enemy_pawn = MakeCell(GetInvertedColor(board.move_side), Piece::Pawn);
            new_tag.score_ -= PSQ[enemy_pawn][taken_coord];
            new_tag.stage_ -= CELL_STAGE_EVAL[enemy_pawn];
        }
        return new_tag;
    } else {
        Q_ASSERT(false);
    }
}

template class Evaluator<EvaluationType::Value>;
template class Evaluator<EvaluationType::Vector>;

}  // namespace q_eval