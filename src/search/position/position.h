#ifndef QUIRKY_SRC_SEARCH_POSITION_POSITION_H
#define QUIRKY_SRC_SEARCH_POSITION_POSITION_H

#include <string_view>

#include "../../core/board/board.h"
#include "../../core/moves/board_manipulation.h"
#include "../../core/moves/movegen.h"
#include "../../eval/evaluator.h"

namespace q_search {

using depth_t = int16_t;
using idepth_t = uint16_t;

struct Position {
    q_core::Board board;
    q_eval::Evaluator<q_eval::EvaluationType::Value> evaluator;

    q_core::Board::FENParseStatus MakeFromFEN(const std::string_view& fen);

    bool MakeMove(q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                  q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag& evaluator_tag);
    void UnmakeMove(q_core::Move move, const q_core::MakeMoveInfo& make_move_info,
                    const q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag& evaluator_tag);

    void MakeNullMove(q_core::coord_t& old_en_passant_coord);
    void UnmakeNullMove(const q_core::coord_t& old_en_passant_coord);

    bool IsCheck() const;

    q_eval::score_t GetEvaluatorScore() const;
    q_eval::ScorePair GetEvaluatorScorePair() const;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_POSITION_H
