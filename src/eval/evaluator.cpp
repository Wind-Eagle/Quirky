#include "evaluator.h"

#include <array>

#include "../core/util.h"
#include "core/board/board.h"
#include "core/board/geometry.h"
#include "core/board/types.h"
#include "eval/score.h"
#include "model.h"
#include "util/bit.h"
#include "util/macro.h"
#include "util/math.h"

using namespace q_core;

namespace q_eval {

void Evaluator::State::Build(const q_core::Board& board) {
    InitializeModelInput(model_input);
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if (board.cells[i] != EMPTY_CELL) {
            UpdateModelInput(model_input, board.cells[i], i, 1);
        }
    }
}

static constexpr score_t LATE_ENDGAME_SCORE_FIX_THRESHOLD = 200;
static constexpr score_t KING_DISTANCE_UNIT_SCORE = 5;
static constexpr score_t KING_CENTER_DISTNACE_UNIT_SCORE = 5;

static constexpr std::array<score_t, BOARD_SIZE> GetKingDistanceBonus() {
    std::array<score_t, BOARD_SIZE> res{};
    for (subcoord_t i = 0; i < BOARD_SIDE / 2; i++) {
        for (subcoord_t j = 0; j < BOARD_SIDE / 2; j++) {
            res[MakeCoord(i, j)] = (i + j - BOARD_SIDE / 2 - 1) * KING_CENTER_DISTNACE_UNIT_SCORE;
            res[MakeCoord(InvertSubcoord(i), j)] =
                (i + j - BOARD_SIDE / 2 - 1) * KING_CENTER_DISTNACE_UNIT_SCORE;
            res[MakeCoord(i, InvertSubcoord(j))] =
                (i + j - BOARD_SIDE / 2 - 1) * KING_CENTER_DISTNACE_UNIT_SCORE;
            res[MakeCoord(InvertSubcoord(i), InvertSubcoord(j))] =
                (i + j - BOARD_SIDE / 2 - 1) * KING_CENTER_DISTNACE_UNIT_SCORE;
        }
    }
    return res;
}

static constexpr std::array<score_t, BOARD_SIZE> KING_DISTANCE_BONUS = GetKingDistanceBonus();

score_t GetLateEndgameBonus(const q_core::Board board, Color c) {
    score_t score = 0;
    if (Q_UNLIKELY(q_util::GetBitCount(board.bb_colors[static_cast<size_t>(c)]) == 1)) {
        q_core::coord_t weak_king_coord =
            q_util::GetLowestBit(board.bb_colors[static_cast<size_t>(c)]);
        q_core::coord_t strong_king_coord =
            q_util::GetLowestBit(board.bb_colors[static_cast<size_t>(GetInvertedColor(c))]);
        score += q_util::GetL1Distance(GetFile(weak_king_coord), GetRank(weak_king_coord),
                                       GetFile(strong_king_coord), GetRank(weak_king_coord)) *
                 KING_DISTANCE_UNIT_SCORE;
        score += KING_DISTANCE_BONUS[weak_king_coord];
    }
    return score;
}

void ApplyLateEndgameBonus(const q_core::Board& board, score_t& score) {
    if (std::abs(score) > LATE_ENDGAME_SCORE_FIX_THRESHOLD) {
        score += GetLateEndgameBonus(board, board.move_side);
        score -= GetLateEndgameBonus(board, GetInvertedColor(board.move_side));
    }
}

score_t Evaluator::Evaluate(const q_core::Board& board) const {
    Q_ASSERT(board.IsValid());
    Q_ASSERT([&]() {
        State state;
        state.Build(board);
        return state == state_;
    }());
    score_t res = ApplyModel(state_.model_input, board.move_side);
    ApplyLateEndgameBonus(board, res);
    return res;
}

void Evaluator::StartTrackingBoard(const q_core::Board& board) { state_.Build(board); }

void Evaluator::UpdateOnMove(const q_core::Board& board, q_core::Move move,
                             EvaluatorUpdateInfo& info) {
    alignas(32) auto new_model_input = state_.model_input;

    const MoveBasicType move_basic_type = GetMoveBasicType(move);
    const auto basic_update = [&](const cell_t src_cell, const cell_t dst_cell) {
        UpdateModelInput(new_model_input, src_cell, move.src, -1);
        UpdateModelInput(new_model_input, dst_cell, move.dst, -1);
    };
    switch (move_basic_type) {
        case MoveBasicType::Simple: {
            basic_update(board.cells[move.src], board.cells[move.dst]);
            UpdateModelInput(new_model_input, board.cells[move.src], move.dst, 1);
            break;
        }
        case MoveBasicType::PawnDouble: {
            const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
            basic_update(pawn, EMPTY_CELL);
            UpdateModelInput(new_model_input, pawn, move.dst, 1);
            break;
        }
        case MoveBasicType::EnPassant: {
            const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
            basic_update(pawn, EMPTY_CELL);
            UpdateModelInput(new_model_input, pawn, move.dst, 1);
            const coord_t taken_coord =
                (board.move_side == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
            const cell_t enemy_pawn = MakeCell(GetInvertedColor(board.move_side), Piece::Pawn);
            UpdateModelInput(new_model_input, enemy_pawn, taken_coord, -1);
            break;
        }
        case MoveBasicType::Castling: {
            const auto king_initial_position = board.move_side == Color::White
                                                   ? WHITE_KING_INITIAL_POSITION
                                                   : BLACK_KING_INITIAL_POSITION;
            if (GetCastlingSide(move) == CastlingSide::Kingside) {
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::King),
                                 king_initial_position + 2, 1);
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::King),
                                 king_initial_position, -1);
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::Rook),
                                 king_initial_position + 1, 1);
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::Rook),
                                 king_initial_position + 3, -1);
            } else {
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::King),
                                 king_initial_position - 2, 1);
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::King),
                                 king_initial_position, -1);
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::Rook),
                                 king_initial_position - 1, 1);
                UpdateModelInput(new_model_input, MakeCell(board.move_side, Piece::Rook),
                                 king_initial_position - 4, -1);
            }
            break;
        }
        case MoveBasicType::KnightPromotion:
        case MoveBasicType::BishopPromotion:
        case MoveBasicType::RookPromotion:
        case MoveBasicType::QueenPromotion: {
            const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
            basic_update(pawn, board.cells[move.dst]);
            cell_t promotion_cell = MakeCell(board.move_side, GetPromotionPiece(move));
            UpdateModelInput(new_model_input, promotion_cell, move.dst, 1);
            break;
        }
        default:
            Q_UNREACHABLE();
    }

    info.old_model_input = state_.model_input;
    state_.model_input = new_model_input;
}

void Evaluator::RevertOnMove(const q_core::Board&, q_core::Move, EvaluatorUpdateInfo& info) {
    state_.model_input = info.old_model_input;
}

}  // namespace q_eval
