#include "position.h"

#include "../../core/moves/attack.h"

namespace q_search {

bool Position::MakeMove(const q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag& evaluator_tag) {
    // TODO: maybe it will be faster to make evaluator update after board make move
    evaluator_tag = evaluator.UpdateOnMove(board, move);
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        evaluator.SetTag(evaluator_tag);
        return false;
    }
    return true;
}

void Position::UnmakeMove(
    const q_core::Move move, const q_core::MakeMoveInfo& make_move_info,
    const q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag& evaluator_tag) {
    q_core::UnmakeMove(board, move, make_move_info);
    evaluator.SetTag(evaluator_tag);
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
    return evaluator.GetEvaluationScore(evaluator.Evaluate(board));
}

q_eval::ScorePair Position::GetEvaluatorScorePair() const { return evaluator.Evaluate(board); }

}  // namespace q_search
