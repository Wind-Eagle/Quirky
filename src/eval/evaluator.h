#ifndef QUIRKY_SRC_EVAL_EVAL_H
#define QUIRKY_SRC_EVAL_EVAL_H

#include "core/board/board.h"
#include "core/moves/board_manipulation.h"
#include "core/moves/move.h"
#include "eval/score.h"
#include "model.h"

namespace q_eval {

class Evaluator {
  public:
    struct State {
        void Build(const q_core::Board& board);
        bool operator==(const State& rhs) const {
            for (size_t i = 0; i < MODEL_INPUT_SIZE; i++) {
                if (std::abs(model_input[i] - rhs.model_input[i]) > 1e-4) {
                    return false;
                }
            }
            return true;
        }

        alignas(64) std::array<int16_t, MODEL_INPUT_SIZE> model_input;
    };

    score_t Evaluate(const q_core::Board& board) const;

    void StartTrackingBoard(const q_core::Board& board, State* state);
    void UpdateOnMove(const q_core::Board& board, q_core::Move move,
                      const q_core::MakeMoveInfo& move_info, State* state);
    void SetState(State* state);

  private:
    alignas(64) State* state_ = nullptr;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_H
