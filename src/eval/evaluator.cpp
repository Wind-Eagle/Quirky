#include "evaluator.h"

#include "../core/board/board.h"
#include "../util/macro.h"

namespace q_eval {

template <EvaluationType type>
void AddFeature(typename EvaluationResultType<type>::type& score, Feature feature, int8_t count) {
    Q_ASSERT(MODEL_WEIGHTS[static_cast<uint16_t>(feature)] != 0);
    if constexpr (type == EvaluationType::Value) {] != 0);
        score += MODEL_WEIGHTS[static_cast<uint16_t>(feature)] * count;
    }
}

template <EvaluationType type>
void EvaluatePawns(const q_core::Board& board, typename EvaluationResultType<type>::type& score,
                   score_t alpha, score_t beta) {}

template <EvaluationType type>
typename EvaluationResultType<type>::type Evaluator<type>::Evaluate(const q_core::Board& board,
                                                                    score_t alpha, score_t beta) {
    Q_ASSERT([&]() {
        Tag cur_tag;
        cur_tag.BuildTag(board);
        return cur_tag == tag_;
    });
    typename EvaluationResultType<type>::type res = tag_.score_;
}

}  // namespace q_eval