#include "evaluator.h"

#include <math.h>

#include "../core/moves/magic.h"
#include "../core/util.h"
#include "eval_features.h"
#include "model.h"
#include "pawns.h"
#include "psq.h"
#include "score.h"

using namespace q_core;

namespace q_eval {

template <EvaluationType type, Color c>
void AddSimpleFeature(typename EvaluationResultType<type>::type& score, const Feature feature,
                      const int8_t count) {
    Q_ASSERT(GetModelFeatureSize(feature) == 1);
    if constexpr (type == EvaluationType::Value) {
        if constexpr (c == Color::White) {
            score += GetModelWeight(feature) * count;
        } else {
            score -= GetModelWeight(feature) * count;
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
void AddArrayFeature(typename EvaluationResultType<type>::type& score, const Feature feature,
                     const uint8_t array_index, const int8_t count) {
    Q_ASSERT(array_index < GetModelFeatureSize(feature) && GetModelFeatureSize(feature) > 1);
    if constexpr (type == EvaluationType::Value) {
        if constexpr (c == Color::White) {
            score += GetModelWeight(feature, array_index) * count;
        } else {
            score -= GetModelWeight(feature, array_index) * count;
        }
    } else {
        if constexpr (c == Color::White) {
            score[static_cast<uint16_t>(feature) + array_index] += count;
        } else {
            score[static_cast<uint16_t>(feature) + array_index] -= count;
        }
    }
}

template <EvaluationType type, Color c>
void EvaluatePawns(const Board& board, typename EvaluationResultType<type>::type& score,
                   uint8_t& open_files_mask) {
    const bitboard_t our_pawns = board.bb_pieces[MakeCell(c, Piece::Pawn)];
    if (!our_pawns) {
        AddSimpleFeature<type, c>(score, Feature::NoPawns, 1);
        open_files_mask = -(static_cast<uint8_t>(0));
        return;
    }

    PawnContext context{.our_pawns = our_pawns, .pawn_coord = UNDEFINED_COORD, .color = c};

    uint8_t pawn_islands_mask = 0;
    bitboard_t pawn_bitboard = board.bb_pieces[MakeCell(c, Piece::Pawn)];
    while (pawn_bitboard) {
        const coord_t pawn_coord = q_util::ExtractLowestBit(pawn_bitboard);
        q_util::SetBit(pawn_islands_mask, GetFile(pawn_coord));
        context.pawn_coord = pawn_coord;

        if (IsPawnIsolated(context)) {
            AddSimpleFeature<type, c>(score, Feature::IsolatedPawn, 1);
        }

        if (IsPawnDoubled(context)) {
            AddSimpleFeature<type, c>(score, Feature::DoubledPawn, 1);
        }
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
    res += score;
    if constexpr (type == EvaluationType::Vector) {
        return ConstructPawnHashTableEntry(ScorePair(), white_open_files_mask,
                                           black_open_files_mask);
    } else {
        return ConstructPawnHashTableEntry(score, white_open_files_mask, black_open_files_mask);
    }
}

template <EvaluationType type, Color c>
void EvaluateNBRQ(const Board& board, typename EvaluationResultType<type>::type& score,
                  const PawnHashTableEntry& pawn_hash_table_entry) {
    const bitboard_t open_files =
        pawn_hash_table_entry.white_open_files_mask & pawn_hash_table_entry.black_open_files_mask;
    const bitboard_t half_open_files =
        (c == Color::White ? pawn_hash_table_entry.white_open_files_mask
                           : pawn_hash_table_entry.black_open_files_mask);
    if (board.bb_pieces[MakeCell(c, Piece::Bishop)]) {
        AddSimpleFeature<type, c>(
            score, Feature::BishopPair,
            q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Bishop)]) - 1);
    }
    AddSimpleFeature<type, c>(score, Feature::RookOnOpenFile,
                              q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Rook)] &
                                                  q_util::ScatterByte(open_files)));
    AddSimpleFeature<type, c>(score, Feature::RookOnHalfOpenFile,
                              q_util::GetBitCount(board.bb_pieces[MakeCell(c, Piece::Rook)] &
                                                  q_util::ScatterByte(half_open_files)));
}

template <EvaluationType type>
void EvaluateNBRQ(const Board& board, typename EvaluationResultType<type>::type& score,
                  const PawnHashTableEntry pawn_hash_table_entry) {
    EvaluateNBRQ<type, Color::White>(board, score, pawn_hash_table_entry);
    EvaluateNBRQ<type, Color::Black>(board, score, pawn_hash_table_entry);
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
    EvaluateNBRQ<type>(board, res, pawn_entry);
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
    ScorePair ans{};
    if constexpr (type == EvaluationType::Vector) {
        const auto& features = score.GetFeatures();
        ans = ApplyModel(features.data(), FEATURE_COUNT);
        for (size_t i = 0; i < PSQ_SIZE; i++) {
            ans += PSQ[i] * features[i + FEATURE_COUNT];
        }
    } else {
        ans = score;
    }
    stage_t stage = std::min(tag_.GetStage(), Tag::STAGE_MAX);
    return (static_cast<int32_t>(ans.GetFirst()) * stage +
            static_cast<int32_t>(ans.GetSecond()) * (Tag::STAGE_MAX - stage)) /
           Tag::STAGE_MAX;
}

template <EvaluationType type>
void Evaluator<type>::Tag::BuildTag(const Board& board) {
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if constexpr (type == EvaluationType::Value) {
            if (board.cells[i] != EMPTY_CELL) {
                score_ += GetPSQValue(board.cells[i], i);
                stage_ += CELL_STAGE_EVAL[board.cells[i]];
            }
        } else {
            if (board.cells[i] != EMPTY_CELL) {
                const uint16_t index = GetPSQIndex(board.cells[i], i);
                score_[FEATURE_COUNT + index]++;
                stage_ += CELL_STAGE_EVAL[board.cells[i]];
            }
        }
    }
}

constexpr std::array<ScorePair, 2> KINGSIGE_CASTLING_PSQ_UPDATE = {
    GetPSQValue(MakeCell(Color::White, Piece::King), WHITE_KING_INITIAL_POSITION + 2) +
        GetPSQValue(MakeCell(Color::White, Piece::Rook), WHITE_KING_INITIAL_POSITION + 1) -
        GetPSQValue(MakeCell(Color::White, Piece::King), WHITE_KING_INITIAL_POSITION) -
        GetPSQValue(MakeCell(Color::White, Piece::Rook), WHITE_KING_INITIAL_POSITION + 3),
    GetPSQValue(MakeCell(Color::Black, Piece::King), BLACK_KING_INITIAL_POSITION + 2) +
        GetPSQValue(MakeCell(Color::Black, Piece::Rook), BLACK_KING_INITIAL_POSITION + 1) -
        GetPSQValue(MakeCell(Color::Black, Piece::King), BLACK_KING_INITIAL_POSITION) -
        GetPSQValue(MakeCell(Color::Black, Piece::Rook), BLACK_KING_INITIAL_POSITION + 3)};

constexpr std::array<ScorePair, 2> QUEENSIGE_CASTLING_PSQ_UPDATE = {
    GetPSQValue(MakeCell(Color::White, Piece::King), WHITE_KING_INITIAL_POSITION - 2) +
        GetPSQValue(MakeCell(Color::White, Piece::Rook), WHITE_KING_INITIAL_POSITION - 1) -
        GetPSQValue(MakeCell(Color::White, Piece::King), WHITE_KING_INITIAL_POSITION) -
        GetPSQValue(MakeCell(Color::White, Piece::Rook), WHITE_KING_INITIAL_POSITION - 4),
    GetPSQValue(MakeCell(Color::Black, Piece::King), BLACK_KING_INITIAL_POSITION - 2) +
        GetPSQValue(MakeCell(Color::Black, Piece::Rook), BLACK_KING_INITIAL_POSITION - 1) -
        GetPSQValue(MakeCell(Color::Black, Piece::King), BLACK_KING_INITIAL_POSITION) -
        GetPSQValue(MakeCell(Color::Black, Piece::Rook), BLACK_KING_INITIAL_POSITION - 4)};

template <EvaluationType type>
typename Evaluator<type>::Tag Evaluator<type>::Tag::GetUpdatedTag(const Board& board,
                                                                  const Move move) const {
    if constexpr (type == EvaluationType::Value) {
        typename Evaluator<type>::Tag new_tag = (*this);
        const MoveBasicType move_basic_type = GetMoveBasicType(move);
        const auto basic_update = [&](const cell_t src_cell, const cell_t dst_cell) {
            new_tag.score_ -= GetPSQValue(src_cell, move.src) + GetPSQValue(dst_cell, move.dst);
            new_tag.stage_ -= CELL_STAGE_EVAL[dst_cell];
        };
        switch (move_basic_type) {
            case MoveBasicType::Simple: {
                basic_update(board.cells[move.src], board.cells[move.dst]);
                new_tag.score_ += GetPSQValue(board.cells[move.src], move.dst);
                break;
            }
            case MoveBasicType::PawnDouble: {
                const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
                basic_update(pawn, EMPTY_CELL);
                new_tag.score_ += GetPSQValue(pawn, move.dst);
                break;
            }
            case MoveBasicType::EnPassant: {
                const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
                basic_update(pawn, EMPTY_CELL);
                new_tag.score_ += GetPSQValue(pawn, move.dst);
                const coord_t taken_coord =
                    (board.move_side == Color::White ? move.dst - BOARD_SIDE
                                                     : move.dst + BOARD_SIDE);
                const cell_t enemy_pawn = MakeCell(GetInvertedColor(board.move_side), Piece::Pawn);
                new_tag.score_ -= GetPSQValue(enemy_pawn, taken_coord);
                break;
            }
            case MoveBasicType::Castling: {
                if (GetCastlingSide(move) == CastlingSide::Kingside) {
                    new_tag.score_ +=
                        KINGSIGE_CASTLING_PSQ_UPDATE[static_cast<uint8_t>(board.move_side)];
                } else {
                    new_tag.score_ +=
                        QUEENSIGE_CASTLING_PSQ_UPDATE[static_cast<uint8_t>(board.move_side)];
                }
                return new_tag;
            }
            case MoveBasicType::KnightPromotion:
            case MoveBasicType::BishopPromotion:
            case MoveBasicType::RookPromotion:
            case MoveBasicType::QueenPromotion: {
                const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
                basic_update(pawn, board.cells[move.dst]);
                cell_t promotion_cell = MakeCell(board.move_side, GetPromotionPiece(move));
                new_tag.score_ += GetPSQValue(promotion_cell, move.dst);
                new_tag.stage_ += CELL_STAGE_EVAL[promotion_cell];
                break;
            }
            default:
                Q_UNREACHABLE();
        }
        return new_tag;
    }
    Q_ASSERT(false);
    return *this;
}

template struct Evaluator<EvaluationType::Value>;
template struct Evaluator<EvaluationType::Vector>;

}  // namespace q_eval