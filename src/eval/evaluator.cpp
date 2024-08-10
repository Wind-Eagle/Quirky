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

template <EvaluationType type>
typename EvaluationResultType<type>::type Evaluator<type>::Evaluate(const Board& board) const {
    Q_ASSERT(board.IsValid());
    Q_ASSERT([&]() {
        Tag cur_tag;
        cur_tag.BuildTag(board);
        return cur_tag == tag_;
    }());
    typename EvaluationResultType<type>::type res = tag_.GetScore();
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