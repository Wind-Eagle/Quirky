#include "position.h"

#include "../../core/moves/attack.h"

namespace q_search {

void Position::UnmakeMove(
    const q_core::Move move, const q_core::MakeMoveInfo& make_move_info,
    q_eval::Evaluator::EvaluatorUpdateInfo& evaluator_update_info) {
    evaluator.RevertOnMove(board, move, evaluator_update_info);
    q_core::UnmakeMove(board, move, make_move_info);
}

bool Position::MakeMove(const q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        q_eval::Evaluator::EvaluatorUpdateInfo& evaluator_update_info) {
    evaluator.UpdateOnMove(board, move, evaluator_update_info);
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        UnmakeMove(move, make_move_info, evaluator_update_info);
        return false;
    }
    return true;
}

void Position::MakeNullMove(q_core::coord_t& old_en_passant_coord) {
    q_core::MakeNullMove(board, old_en_passant_coord);
}

void Position::UnmakeNullMove(const q_core::coord_t& old_en_passant_coord) {
    q_core::UnmakeNullMove(board, old_en_passant_coord);
}

bool Position::IsCheck() const {
    return q_core::IsKingInCheck(board);
}

q_core::Board::FENParseStatus Position::MakeFromFEN(const std::string_view& fen) {
    auto res = board.MakeFromFEN(fen);
    if (res != q_core::Board::FENParseStatus::Ok) {
        return res;
    }
    evaluator.StartTrackingBoard(board);
    return q_core::Board::FENParseStatus::Ok;
}

q_eval::score_t Position::GetEvaluatorScore() const {
    return evaluator.Evaluate(board);
}

}  // namespace q_search
