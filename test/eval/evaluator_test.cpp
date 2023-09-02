#include <gtest/gtest.h>

#include "../../src/eval/evaluator.h"

void SanityCheck(const std::string_view& str, q_eval::score_t min, q_eval::score_t max) {
    q_core::Board board;
    board.MakeFromFEN(str);
    q_eval::Evaluator<q_eval::EvaluationType::Vector> evaluator;
    evaluator.StartTrackingBoard(board);
    auto evaluation = evaluator.Evaluate(board);
    q_eval::score_t score = evaluator.GetEvaluationScore(evaluator.Evaluate(board));
    const auto& ans = evaluation.GetFeatures();
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        if (ans[i] != 0) {
            q_util::Print(i, ans[i]);
        }
    }
    std::cout << std::endl;
    EXPECT_LE(min, score);
    EXPECT_LE(score, max);
}

TEST(EvalEvaluator, SanityCheck) {
    // SanityCheck("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, 50);
    // SanityCheck("4K3/8/8/7R/8/8/8/4k3 w - - 0 1", 100, q_eval::SCORE_MAX);
    SanityCheck("r1b2rk1/ppq1ppbp/2n3p1/1B1pP3/5B2/2P5/PP2QPPP/RN3RK1 w - - 6 12", -1000, 1000);
}