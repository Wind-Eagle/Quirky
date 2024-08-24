#ifndef QUIRKY_TOOLS_EVAL_TUNER_STORAGE_DATASET_H
#define QUIRKY_TOOLS_EVAL_TUNER_STORAGE_DATASET_H

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>

#include "../../../src/core/board/board.h"
#include "../../../src/eval/model.h"
#include "reader.h"

enum PositionScoreType {
    NotReady = 0,
    UpdatedJustBefore = 1,
    Ready = 2
};

struct PositionState {
    q_eval::score_t old_score;
    q_eval::score_t new_score;
    PositionScoreType score_type;
    bool force_update;
};

struct Element {
    Game game;
    std::vector<PositionState> states;
};

class Dataset {
  public:
    void Load(Reader& reader);
    std::vector<std::shared_ptr<Element>> GetAllElements() const;
    size_t Size() const;
  private:
    std::vector<std::shared_ptr<Element>> elements_{};
    std::mutex mutex_;
};

#endif  // QUIRKY_TOOLS_EVAL_TUNER_STORAGE_DATASET_H
