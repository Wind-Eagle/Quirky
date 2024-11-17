#ifndef QUIRKY_SRC_EVAL_EVAL_H
#define QUIRKY_SRC_EVAL_EVAL_H

#include "../core/board/board.h"
#include "../core/moves/board_manipulation.h"
#include "../core/moves/move.h"
#include "../core/util.h"

#include "model.h"

#include <vector>

namespace q_eval {

class Evaluator {
  public:
    struct EvaluatorUpdateInfo {
        std::array<int16_t, FEATURE_LAYER_SIZE> old_feature_layer;
        stage_t old_stage;
    };

    score_t Evaluate(const q_core::Board& board) const;

    void StartTrackingBoard(const q_core::Board& board);
    void UpdateOnMove(const q_core::Board& board, q_core::Move move, EvaluatorUpdateInfo& info);
    void RevertOnMove(const q_core::Board& board, q_core::Move move, EvaluatorUpdateInfo& info);

  private:
    static constexpr stage_t CELL_STAGE_EVAL[q_core::NUMBER_OF_CELLS] = {0, 0, 1, 1, 2, 4, 0,
                                                                             0, 1, 1, 2, 4, 0};
    struct State {
      void Build(const q_core::Board& board);
      bool operator==(const State& rhs) const {
        return feature_layer == rhs.feature_layer && stage == rhs.stage;
      }
      
      std::array<int16_t, FEATURE_LAYER_SIZE> feature_layer;
      stage_t stage;
    };
    State state_;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_H
