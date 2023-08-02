#include "evaluator.h"

#include "../core/board/board.h"
#include "../util/macro.h"

namespace q_eval {

template <EvaluationType type>
void AddFeature(typename EvaluationResultType<type>::type& score, Feature feature, int8_t count) {
    Q_ASSERT(MODEL_WEIGHTS[static_cast<uint16_t>(feature)] != 0);
    if constexpr (type == EvaluationType::Value) {
        score += MODEL_WEIGHTS[static_cast<uint16_t>(feature)] * count;
    }
}

template <EvaluationType type>
typename EvaluationResultType<type>::type Evaluator<type>::Evaluate(const q_core::Board& board,
                                                                    score_t alpha, score_t beta) {
    Q_ASSERT([&]() {
        Tag cur_tag;
        cur_tag.BuildTag(board);
        return cur_tag == tag;
    });
}

}  // namespace q_eval