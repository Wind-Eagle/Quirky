#include "evaluator.h"

#include "../core/util.h"
#include "model.h"

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

score_t Evaluator::Evaluate(const q_core::Board& board) const {
    Q_ASSERT(board.IsValid());
    Q_ASSERT([&]() {
        State state;
        state.Build(board);
        return state == state_;
    }());
    score_t res = ApplyModel(state_.model_input);
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

void Evaluator::RevertOnMove(const q_core::Board&,
                             q_core::Move,
                             EvaluatorUpdateInfo& info) {
    state_.model_input = info.old_model_input;
}

}  // namespace q_eval
