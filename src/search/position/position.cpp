#include "position.h"

namespace q_search {

bool Position::MakeMove(const q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag& evaluator_tag) {
    // TODO: rewrite this to make evaluator update after board make move
    evaluator_tag = evaluator.UpdateOnMove(board, move);
    bool is_move_legal = q_core::MakeMove(board, move, make_move_info);
    if (!is_move_legal) {
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

}  // namespace q_search
