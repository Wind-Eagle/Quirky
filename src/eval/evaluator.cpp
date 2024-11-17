#include "evaluator.h"

#include "../core/util.h"
#include "model.h"

using namespace q_core;

namespace q_eval {

void Evaluator::State::Build(const q_core::Board& board) {
    feature_layer.fill(0);
    stage = 0;
    auto& feature_layer_array = feature_layer;
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if (board.cells[i] != EMPTY_CELL) {
            UpdateFeatureLayer(feature_layer_array, board.cells[i], i, 1);
        }
        stage += CELL_STAGE_EVAL[board.cells[i]];
    }
}

score_t Evaluator::Evaluate(const q_core::Board& board) const {
    Q_ASSERT(board.IsValid());
    Q_ASSERT([&]() {
        State state;
        state.Build(board);
        return state == state_;
    }());
    score_t res = ApplyModel(state_.feature_layer, state_.stage);
    if (board.move_side == Color::Black) {
        res *= -1;
    }
    return res;
}

void Evaluator::StartTrackingBoard(const q_core::Board& board) {
    state_.Build(board);
}

void Evaluator::UpdateOnMove(const q_core::Board& board,
                             q_core::Move move,
                             EvaluatorUpdateInfo& info) {
    auto new_feature_layer = state_.feature_layer;
    auto& new_feature_layer_array = new_feature_layer;
    stage_t new_stage = state_.stage;

    const MoveBasicType move_basic_type = GetMoveBasicType(move);
    const auto basic_update = [&](const cell_t src_cell, const cell_t dst_cell) {
        UpdateFeatureLayer(new_feature_layer_array, src_cell, move.src, -1);
        UpdateFeatureLayer(new_feature_layer_array, dst_cell, move.dst, -1);
        new_stage -= CELL_STAGE_EVAL[dst_cell];
    };
    switch (move_basic_type) {
        case MoveBasicType::Simple: {
            basic_update(board.cells[move.src], board.cells[move.dst]);
            UpdateFeatureLayer(new_feature_layer_array, board.cells[move.src], move.dst, 1);
            break;
        }
        case MoveBasicType::PawnDouble: {
            const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
            basic_update(pawn, EMPTY_CELL);
            UpdateFeatureLayer(new_feature_layer_array, pawn, move.dst, 1);
            break;
        }
        case MoveBasicType::EnPassant: {
            const cell_t pawn = MakeCell(board.move_side, Piece::Pawn);
            basic_update(pawn, EMPTY_CELL);
            UpdateFeatureLayer(new_feature_layer_array, pawn, move.dst, 1);
            const coord_t taken_coord =
                (board.move_side == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
            const cell_t enemy_pawn = MakeCell(GetInvertedColor(board.move_side), Piece::Pawn);
            UpdateFeatureLayer(new_feature_layer_array, enemy_pawn, taken_coord, -1);
            break;
        }
        case MoveBasicType::Castling: {
            const auto king_initial_position = board.move_side == Color::White
                                                   ? WHITE_KING_INITIAL_POSITION
                                                   : BLACK_KING_INITIAL_POSITION;
            if (GetCastlingSide(move) == CastlingSide::Kingside) {
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::King),
                                   king_initial_position + 2, 1);
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::King),
                                   king_initial_position, -1);
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::Rook),
                                   king_initial_position + 1, 1);
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::Rook),
                                   king_initial_position + 3, -1);
            } else {
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::King),
                                   king_initial_position - 2, 1);
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::King),
                                   king_initial_position, -1);
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::Rook),
                                   king_initial_position - 1, 1);
                UpdateFeatureLayer(new_feature_layer_array, MakeCell(board.move_side, Piece::Rook),
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
            UpdateFeatureLayer(new_feature_layer_array, promotion_cell, move.dst, 1);
            new_stage += CELL_STAGE_EVAL[promotion_cell];
            break;
        }
        default:
            Q_UNREACHABLE();
    }

    info.old_feature_layer = state_.feature_layer;
    info.old_stage = state_.stage;
    state_.feature_layer = new_feature_layer;
    state_.stage = new_stage;
}

void Evaluator::RevertOnMove(const q_core::Board&,
                             q_core::Move,
                             EvaluatorUpdateInfo& info) {
    state_.feature_layer = info.old_feature_layer;
    state_.stage = info.old_stage;
}

}  // namespace q_eval
