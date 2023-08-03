#include <gtest/gtest.h>

#include "../../src/eval/evaluator.h"

void SanityCheck(const std::string_view& str, q_eval::score_t min, q_eval::score_t max) {
    q_core::Board board;
    board.MakeFromFEN(str);
    q_eval::Evaluator<q_eval::EvaluationType::Value> evaluator;
    evaluator.StartTrackingBoard(board);
    q_eval::score_t evaluation = evaluator.Evaluate(board);
    EXPECT_LE(min, evaluation);
    EXPECT_LE(evaluation, max);
}

TEST(EvalEvaluator, SanityCheck) {
    SanityCheck("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, 50);
    SanityCheck("4K3/8/8/7R/8/8/8/4k3 w - - 0 1", 100, q_eval::SCORE_MAX);

}