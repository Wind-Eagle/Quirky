#ifndef QUIRKY_SRC_EVAL_EVAL_H
#define QUIRKY_SRC_EVAL_EVAL_H

#include "../core/board/board.h"
#include "../core/moves/move.h"
#include "core/moves/board_manipulation.h"
#include "model.h"
#include "util/arena.h"

namespace q_eval {

class Evaluator {
  public:
    typedef q_util::Arena<std::array<int16_t, MODEL_INPUT_SIZE>, 4096, 64> model_input_arena;

    struct EvaluatorUpdateInfo {
        EvaluatorUpdateInfo(model_input_arena* arena) : arena(arena) {
            old_model_input = arena->Allocate();
        }

        ~EvaluatorUpdateInfo() { arena->Deallocate(old_model_input); }

        __attribute((aligned(64))) std::array<int16_t, MODEL_INPUT_SIZE>* old_model_input;
        model_input_arena* arena;
    };

    Evaluator() : arena_(), state_(&arena_) {}

    model_input_arena* Arena() { return &arena_; }
    score_t Evaluate(const q_core::Board& board);

    void StartTrackingBoard(const q_core::Board& board);
    void UpdateOnMove(const q_core::Board& board, q_core::Move move,
                      const q_core::MakeMoveInfo& move_info, EvaluatorUpdateInfo& info);
    void RevertOnMove(const q_core::Board& board, q_core::Move move, EvaluatorUpdateInfo& info);

  private:
    struct State {
        State(model_input_arena* arena) : arena(arena) { model_input = arena->Allocate(); }

        ~State() { arena->Deallocate(model_input); }

        void Build(const q_core::Board& board);
        bool operator==(const State& rhs) const {
            for (size_t i = 0; i < MODEL_INPUT_SIZE; i++) {
                if (std::abs((*model_input)[i] - (*rhs.model_input)[i]) > 1e-4) {
                    return false;
                }
            }
            return true;
        }

        __attribute((aligned(64))) std::array<int16_t, MODEL_INPUT_SIZE>* model_input;
        model_input_arena* arena;
    };

    model_input_arena arena_;
    State state_;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_H
