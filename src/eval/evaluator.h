#ifndef QUIRKY_SRC_EVAL_EVAL_H
#define QUIRKY_SRC_EVAL_EVAL_H

#include "../core/board/board.h"
#include "../core/moves/move.h"
#include "model.h"

namespace q_eval {

enum class EvaluationType : uint8_t { Value = 0, Vector = 1 };

template <EvaluationType t>
struct EvaluationResultType {
    typedef score_t type;
};

template <>
struct EvaluationResultType<EvaluationType::Vector> {
    typedef std::array<score_t, FEATURE_COUNT> type;
};

template <EvaluationType type>
struct Evaluator {
  public:
    struct Tag {
      public:
        void BuildTag(const q_core::Board& board);
        Tag UpdateTag(const q_core::Board& board, const q_core::Move move);
      private:
        typename EvaluationResultType<type>::type score_;
        stage_t stage_;
    };

    void StartTrackingBoard(const q_core::Board& board) { tag_.BuildTag(board);
    }

    void UpdateOnMove(const q_core::Board& board, const q_core::Move move) {
        tag_.UpdateTag(board, move);
    }

    void ReplaceTag(const Tag new_tag) { tag_ = new_tag;
    }

    typename EvaluationResultType<type>::type Evaluate(const q_core::Board& board,
                                                       score_t alpha = SCORE_MIN,
                                                       score_t beta = SCORE_MAX);

  private:
    Tag tag_;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_H
