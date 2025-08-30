#include "evaluator.h"

#include <array>

#include "core/util.h"
#include "core/board/board.h"
#include "core/board/geometry.h"
#include "core/board/types.h"
#include "core/moves/board_manipulation.h"
#include "core/moves/move.h"
#include "eval/score.h"
#include "model.h"
#include "util/macro.h"

using namespace q_core;

namespace q_eval {

void Evaluator::State::Build(const q_core::Board& board) {
    InitializeModelInput(model_input);
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if (board.cells[i] != EMPTY_CELL) {
            Add(model_input, board.cells[i], i);
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
    score_t res = ApplyModel(state_.model_input, board.move_side);
    return res;
}

void Evaluator::StartTrackingBoard(const q_core::Board& board) { state_.Build(board); }

void Evaluator::UpdateOnMove(const q_core::Board& board, q_core::Move move,
                             const q_core::MakeMoveInfo& move_info, EvaluatorUpdateInfo& info) {
    info.old_model_input = state_.model_input;
    alignas(64) auto& new_model_input = state_.model_input;
    const q_core::Color move_side = q_core::GetInvertedColor(board.move_side);

    const MoveBasicType move_basic_type = GetMoveBasicType(move);
    switch (move_basic_type) {
        case MoveBasicType::Simple: {
            if (move_info.dst_cell == EMPTY_CELL) {
                SubAdd(new_model_input, board.cells[move.dst], move.src, board.cells[move.dst], move.dst);
            } else {
                SubSubAdd(new_model_input, board.cells[move.dst], move.src, move_info.dst_cell, move.dst, board.cells[move.dst], move.dst);
            }
            break;
        }
        case MoveBasicType::PawnDouble: {
            const cell_t pawn = MakeCell(move_side, Piece::Pawn);
            SubAdd(new_model_input, pawn, move.src, pawn, move.dst);
            break;
        }
        case MoveBasicType::EnPassant: {
            const cell_t pawn = MakeCell(move_side, Piece::Pawn);
            const coord_t taken_coord =
                (move_side == Color::White ? move.dst - BOARD_SIDE : move.dst + BOARD_SIDE);
            const cell_t enemy_pawn = MakeCell(GetInvertedColor(move_side), Piece::Pawn);
            SubSubAdd(new_model_input, pawn, move.src, enemy_pawn, taken_coord, pawn, move.dst);
            break;
        }
        case MoveBasicType::Castling: {
            const auto king_initial_position = move_side == Color::White
                                                   ? WHITE_KING_INITIAL_POSITION
                                                   : BLACK_KING_INITIAL_POSITION;
            const cell_t king = MakeCell(move_side, Piece::King);
            const cell_t rook = MakeCell(move_side, Piece::Rook);
            if (GetCastlingSide(move) == CastlingSide::Kingside) {
                SubAdd(new_model_input, king, king_initial_position, king, king_initial_position + 2);
                SubAdd(new_model_input, rook, king_initial_position + 3, rook, king_initial_position + 1);
            } else {
                SubAdd(new_model_input, king, king_initial_position, king, king_initial_position - 2);
                SubAdd(new_model_input, rook, king_initial_position - 4, rook, king_initial_position - 1);
            }
            break;
        }
        case MoveBasicType::KnightPromotion:
        case MoveBasicType::BishopPromotion:
        case MoveBasicType::RookPromotion:
        case MoveBasicType::QueenPromotion: {
            const cell_t pawn = MakeCell(move_side, Piece::Pawn);
            cell_t promotion_cell = MakeCell(move_side, GetPromotionPiece(move));
            if (move_info.dst_cell == EMPTY_CELL) {
                SubAdd(new_model_input, pawn, move.src, promotion_cell, move.dst);
            } else {
                SubSubAdd(new_model_input, pawn, move.src, move_info.dst_cell, move.dst, promotion_cell, move.dst);
            }
            break;
        }
        default:
            Q_UNREACHABLE();
    }
}

void Evaluator::RevertOnMove(const q_core::Board&, q_core::Move, EvaluatorUpdateInfo& info) {
    state_.model_input = info.old_model_input;
}

}  // namespace q_eval
