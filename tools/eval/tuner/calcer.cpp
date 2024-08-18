#include "calcer.h"

#include "../../../src/search/position/position.h"
#include "../../../src/search/position/move_picker.h"
#include "reader.h"

#include <cmath>

double GetProbabilityFromScore(const q_eval::score_t score) {
    constexpr double PROB_SIGMOID_SCALE = 1.0475;
    return 1.0 / (1 + std::pow(10, PROB_SIGMOID_SCALE * -score / 400.0));
}

#define MAKE_MOVE(position, move)                                           \
    q_core::MakeMoveInfo _make_move_info;                                   \
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag _evaluator_tag;   \
    bool _legal = position.MakeMove(move, _make_move_info, _evaluator_tag); \
    if (!_legal) {                                                          \
        continue;                                                           \
    }                                                                       \
    Q_DEFER { position.UnmakeMove(move, _make_move_info, _evaluator_tag); }

q_eval::score_t FakeQuiescenseSearch(q_search::Position& position, q_eval::score_t alpha, q_eval::score_t beta) {
    const q_eval::score_t score = position.GetEvaluatorScore();
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
        return beta;
    }
    q_search::QuiescenseMovePicker move_picker(position);
    for (q_core::Move move = move_picker.GetNextMove();
         move_picker.GetStage() != q_search::QuiescenseMovePicker::Stage::End;
         move = move_picker.GetNextMove()) {
        MAKE_MOVE(position, move);
        q_eval::score_t new_score = -FakeQuiescenseSearch(position, -beta, -alpha);
        alpha = std::max(alpha, new_score);
        if (alpha >= beta) {
            return beta;
        }
    }
    return alpha;
}

CalcerResult GetCalcerResult(Game& game) {
    q_search::Position position;
    position.MakeFromFEN(game.start_board_fen);
    double target = game.header.result == Result::WhiteWins ? 1 : game.header.result == Result::BlackWins ? 0 : 0.5;
    double loss = 0;

    q_core::MakeMoveInfo make_move_info;
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag evaluator_tag;
    size_t position_count = 0;
    for (size_t i = 0; i < game.moves.size(); i++) {
        if (i > 0) {
            position.MakeMove(q_core::TranslateStringToMove(position.board, game.moves[i]), make_move_info, evaluator_tag);
        }
        if (position.board.move_count > 12) {
            q_eval::score_t score = FakeQuiescenseSearch(position, q_eval::SCORE_MIN, q_eval::SCORE_MAX);
            if (position.board.move_side == q_core::Color::Black) {
                score *= -1;
            }
            const double prob = GetProbabilityFromScore(score);
            loss += (prob - target) * (prob - target);
            position_count++;
        }
    }
    return {.loss = loss, .number_of_positions = position_count};
}
