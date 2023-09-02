#include "evaluator.h"

#include "../core/moves/magic.h"
#include "../core/util.h"
#include "eval_features.h"
#include "pawns.h"
#include "psq.h"

namespace q_eval {

template <EvaluationType type, q_core::Color c>
void AddFeature(typename EvaluationResultType<type>::type& score, const Feature feature,
                const int8_t count) {
    /*Q_ASSERT(MODEL_WEIGHTS[static_cast<uint16_t>(feature)].GetFirst() != 0 ||
             MODEL_WEIGHTS[static_cast<uint16_t>(feature)].GetSecond() != 0);*/
    if constexpr (type == EvaluationType::Value) {
        if constexpr (c == q_core::Color::White) {
            score += MODEL_WEIGHTS[static_cast<uint16_t>(feature)] * count;
        } else {
            score -= MODEL_WEIGHTS[static_cast<uint16_t>(feature)] * count;
        }
    } else {
        if constexpr (c == q_core::Color::White) {
            score[static_cast<uint16_t>(feature)] += count;
        } else {
            score[static_cast<uint16_t>(feature)] -= count;
        }
    }
}

template <EvaluationType type, q_core::Color c>
void EvaluatePawns(const q_core::Board& board, typename EvaluationResultType<type>::type& score,
                   uint8_t& open_files_mask) {
    const q_core::bitboard_t our_pawns = board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Pawn)];
    const q_core::bitboard_t enemy_pawns =
        board.bb_pieces[q_core::MakeCell(q_core::GetInvertedColor(c), q_core::Piece::Pawn)];

    const auto colored_pawn_frontspan =
        (c == q_core::Color::White ? WHITE_PAWN_FRONTSPAN_BITBOARD : BLACK_PAWN_FRONTSPAN_BITBOARD);

    uint8_t pawn_islands_mask = 0;
    q_core::bitboard_t pawn_bitboard = board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Pawn)];
    while (pawn_bitboard) {
        const q_core::coord_t pawn_coord = q_util::ExtractLowestBit(pawn_bitboard);
        const q_core::subcoord_t pawn_rank = q_core::GetRank(pawn_coord);
        const q_core::subcoord_t pawn_file = q_core::GetFile(pawn_coord);
        q_util::SetBit(pawn_islands_mask, pawn_file);
        const q_core::subcoord_t relative_pawn_rank =
            (c == q_core::Color::White ? pawn_rank : q_core::InvertSubcoord(pawn_rank));

        bool is_pawn_isolated = false;
        bool is_pawn_passed = false;

        if ((c == q_core::Color::White ? q_core::WHITE_PAWN_REVERSED_ATTACK_BITBOARD[pawn_coord]
                                       : q_core::BLACK_PAWN_REVERSED_ATTACK_BITBOARD[pawn_coord]) &
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

            if (!((c == q_core::Color::White ? WHITE_PAWN_PASSED_BITBOARD[pawn_coord]
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

        if (!((c == q_core::Color::White ? (~(BLACK_PAWN_PASSED_BITBOARD[pawn_coord]))
                                         : (~(WHITE_PAWN_PASSED_BITBOARD[pawn_coord]))) |
              PAWN_CONNECTED_BITBOARD[pawn_coord] & (~q_core::FILE_BITBOARD[pawn_rank]) &
                  our_pawns) &&
            ((c == q_core::Color::White ? WHITE_PAWN_BACKWARD_SENTRY_BITBOARD[pawn_coord]
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
PawnHashTableEntry EvaluatePawns(const q_core::Board& board, typename EvaluationResultType<type>::type& res) {
    uint8_t white_open_files_mask = 0;
    uint8_t black_open_files_mask = 0;
    typename EvaluationResultType<type>::type score{};
    EvaluatePawns<type, q_core::Color::White>(board, score, white_open_files_mask);
    EvaluatePawns<type, q_core::Color::Black>(board, score, black_open_files_mask);
    if constexpr (type == EvaluationType::Vector) {
        return PawnHashTableEntry{ScorePair(), white_open_files_mask, black_open_files_mask};
    } else {
        return PawnHashTableEntry{score, white_open_files_mask, black_open_files_mask};
    }
}

template <EvaluationType type, q_core::Color c>
void EvaluateKNBRQ(const q_core::Board& board, typename EvaluationResultType<type>::type& score,
                   const PawnHashTableEntry pawn_hash_table_entry) {
    q_core::bitboard_t open_files = pawn_hash_table_entry.white_half_open_files_mask &
                                    pawn_hash_table_entry.black_half_open_files_mask;
    q_core::bitboard_t half_open_files =
        (c == q_core::Color::White ? ~pawn_hash_table_entry.white_half_open_files_mask
                                   : ~pawn_hash_table_entry.black_half_open_files_mask);
    if (board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Knight)]) {
        AddFeature<type, c>(
            score, Feature::KnightPair,
            q_util::GetBitCount(board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Knight)]) - 1);
    }
    if (board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Bishop)]) {
        AddFeature<type, c>(
            score, Feature::BishopPair,
            q_util::GetBitCount(board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Bishop)]) - 1);
    }
    AddFeature<type, c>(
        score, Feature::RookOnOpenFile,
        q_util::GetBitCount(board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Rook)] &
                            q_util::ScatterByte(open_files)));
    AddFeature<type, c>(
        score, Feature::RookOnHalfOpenFile,
        q_util::GetBitCount(board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Rook)] &
                            q_util::ScatterByte(half_open_files)));
    const q_core::coord_t enemy_king_pos = q_util::GetLowestBit(
        board.bb_pieces[q_core::MakeCell(q_core::GetInvertedColor(c), q_core::Piece::King)]);
    if (board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Queen)] != 0) {
        const q_core::coord_t queen_pos =
            q_util::GetLowestBit(board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Queen)]);
        const uint8_t queen_king_distance = GetLInftyDistance(enemy_king_pos, queen_pos);
        if (queen_king_distance >= QUEEN_KING_DISTANCE_FEATURE_FIRST &&
            queen_king_distance <= QUEEN_KING_DISTANCE_FEATURE_LAST) {
            AddFeature<type, c>(
                score,
                static_cast<Feature>(static_cast<uint16_t>(Feature::QueenKingDistance1) +
                                     queen_king_distance - QUEEN_KING_DISTANCE_FEATURE_FIRST),
                1);
        }
    }
    const q_core::coord_t king_pos =
        q_util::GetLowestBit(board.bb_pieces[q_core::MakeCell(c, q_core::Piece::King)]);
    const q_core::subcoord_t king_rank = q_core::GetRank(king_pos);
    const q_core::subcoord_t king_file = q_core::GetFile(king_pos);
    if (!(c == q_core::Color::White
              ? q_core::IsAnyCastlingAllowed(board.castling & q_core::Castling::WhiteAll)
              : q_core::IsAnyCastlingAllowed(board.castling & q_core::Castling::BlackAll)) &&
        (c == q_core::Color::White ? WHITE_KING_SHIELDED_BITBOARD : BLACK_KING_SHIELDED_BITBOARD) &
            q_core::MakeBitboardFromCoord(king_pos)) {
        constexpr int8_t DIR = (c == q_core::Color::White ? 1 : -1);
        const q_core::bitboard_t our_pawns =
            board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Pawn)];
        const q_core::bitboard_t enemy_pawns =
            board.bb_pieces[q_core::MakeCell(q_core::GetInvertedColor(c), q_core::Piece::Pawn)];
        const uint8_t shield_mask1 =
            ((our_pawns & q_core::RANK_BITBOARD[king_rank + DIR]) >>
             (c == q_core::Color::White ? q_core::BOARD_SIDE - 1 + king_pos
                                        : king_pos - q_core::BOARD_SIDE - 1)) &
            7;
        const uint8_t shield_mask2 =
            ((our_pawns & q_core::RANK_BITBOARD[king_rank + DIR * 2]) >>
             (c == q_core::Color::White ? q_core::BOARD_SIDE * 2 - 1 + king_pos
                                        : king_pos - q_core::BOARD_SIDE * 2 - 1)) &
            7;
        const uint8_t storm_mask2 =
            ((enemy_pawns & q_core::RANK_BITBOARD[king_rank + DIR * 2]) >>
             (c == q_core::Color::White ? q_core::BOARD_SIDE * 2 - 1 + king_pos
                                        : king_pos - q_core::BOARD_SIDE * 2 - 1)) &
            7;
        const uint8_t storm_mask3 =
            ((enemy_pawns & q_core::RANK_BITBOARD[king_rank + DIR * 3]) >>
             (c == q_core::Color::White ? q_core::BOARD_SIDE * 3 - 1 + king_pos
                                        : king_pos - q_core::BOARD_SIDE * 3 - 1)) &
            7;
        const bool inverted = king_file >= q_core::BOARD_SIDE / 2;
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
void EvaluateKNBRQ(const q_core::Board& board, typename EvaluationResultType<type>::type& score,
                   const PawnHashTableEntry pawn_hash_table_entry) {
    EvaluateKNBRQ<type, q_core::Color::White>(board, score, pawn_hash_table_entry);
    EvaluateKNBRQ<type, q_core::Color::Black>(board, score, pawn_hash_table_entry);
}

template <EvaluationType type>
typename EvaluationResultType<type>::type Evaluator<type>::Evaluate(const q_core::Board& board) const {
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
        if (board.move_side == q_core::Color::Black) {
            res *= -1;
        }
    }
    return res;
}

template <EvaluationType type>
score_t Evaluator<type>::GetEvaluationScore(const typename EvaluationResultType<type>::type score) const {
    ScorePair ans = 0;
    if constexpr (type == EvaluationType::Vector) {
        const auto& features = score.GetFeatures();
        for (size_t i = 0; i < FEATURE_COUNT; i++) {
            ans += MODEL_WEIGHTS[i];
        }
        for (size_t i = 0; i < PSQ_SIZE; i++) {
            ans += PSQ[i / q_core::BOARD_SIZE][i % q_core::BOARD_SIZE];
        }
    } else {
        ans = score;
    }
    stage_t stage = std::max(tag_.GetStage(), Tag::STAGE_MAX);
    return (ans.GetFirst() * stage + ans.GetSecond() * (Tag::STAGE_MAX - stage)) / Tag::STAGE_MAX;
}

template <EvaluationType type>
void Evaluator<type>::Tag::BuildTag(const q_core::Board& board) {
    for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
        if constexpr (type == EvaluationType::Value) {
            if (board.cells[i] != q_core::EMPTY_CELL) {
                if (q_core::GetCellColor(board.cells[i]) == q_core::Color::White) {
                    score_ += PSQ[board.cells[i]][i];
                } else {
                    score_ -= PSQ[board.cells[i]][i];
                }
                stage_ += CELL_STAGE_EVAL[board.cells[i]];
            }
        } else {
            const uint16_t index = GetPSQIndex(board.cells[i], i);
            if (board.cells[i] != q_core::EMPTY_CELL) {
                if (q_core::GetCellColor(board.cells[i]) == q_core::Color::White) {
                    score_[FEATURE_COUNT + index]++;
                } else {
                    score_[FEATURE_COUNT + index]--;
                }
                stage_ += CELL_STAGE_EVAL[board.cells[i]];
            }
        }
    }
}

constexpr std::array<ScorePair, 2> KINGSIGE_CASTLING_PSQ_UPDATE =
    {PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::WHITE_KING_INITIAL_POSITION + 2] +
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::WHITE_KING_INITIAL_POSITION + 1] -
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::WHITE_KING_INITIAL_POSITION] -
     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::WHITE_KING_INITIAL_POSITION + 3],
     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::BLACK_KING_INITIAL_POSITION + 2] +
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::BLACK_KING_INITIAL_POSITION + 1] -
     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::BLACK_KING_INITIAL_POSITION] -
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::BLACK_KING_INITIAL_POSITION + 3]};

constexpr std::array<ScorePair, 2> QUEENSIGE_CASTLING_PSQ_UPDATE =
    {     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::WHITE_KING_INITIAL_POSITION - 2] +
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::WHITE_KING_INITIAL_POSITION - 1] -
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::WHITE_KING_INITIAL_POSITION] -
     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::WHITE_KING_INITIAL_POSITION - 4],
     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::BLACK_KING_INITIAL_POSITION - 2] +
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::BLACK_KING_INITIAL_POSITION - 1] -
     PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::King)][q_core::BLACK_KING_INITIAL_POSITION] -
         PSQ[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)][q_core::BLACK_KING_INITIAL_POSITION - 4]};

template <EvaluationType type>
typename Evaluator<type>::Tag Evaluator<type>::Tag::UpdateTag(const q_core::Board& board, const q_core::Move move) {
    Q_ASSERT(!IsMoveNull(move) && !IsMoveUndefined(move));
    if constexpr (type == EvaluationType::Value) {
        typename Evaluator<type>::Tag new_tag = (*this);
        const q_core::cell_t src_cell = board.cells[move.src];
        const q_core::cell_t dst_cell = board.cells[move.dst];
        if (q_core::IsMoveCastling(move)) {
            if (move.type == q_core::KINGSIDE_CASTLING_MOVE_TYPE) {
                new_tag.score_ += KINGSIGE_CASTLING_PSQ_UPDATE[static_cast<uint8_t>(board.move_side)];
            } else {
                new_tag.score_ += QUEENSIGE_CASTLING_PSQ_UPDATE[static_cast<uint8_t>(board.move_side)];
            }
            return new_tag;
        }
        new_tag.score_ -= PSQ[src_cell][move.src] + PSQ[dst_cell][move.dst];
        new_tag.stage_ -= CELL_STAGE_EVAL[dst_cell];
        if (q_core::IsMovePromotion(move)) {
            q_core::cell_t promotion_cell =
                q_core::MakeCell(board.move_side, q_core::GetPromotionPiece(move));
            new_tag.score_ += PSQ[promotion_cell][move.dst];
            new_tag.stage_ += CELL_STAGE_EVAL[promotion_cell];
            return new_tag;
        }
        new_tag.score_ += PSQ[src_cell][move.dst];
        if (q_core::IsMoveEnPassant(move)) {
            const q_core::coord_t taken_coord =
                (board.move_side == q_core::Color::White ? move.dst - q_core::BOARD_SIDE
                                                         : move.dst + q_core::BOARD_SIDE);
            const q_core::cell_t enemy_pawn =
                q_core::MakeCell(q_core::GetInvertedColor(board.move_side), q_core::Piece::Pawn);
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