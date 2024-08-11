#ifndef QUIRKY_TOOLS_EVAL_TUNER_STORAGE_DATASET_H
#define QUIRKY_TOOLS_EVAL_TUNER_STORAGE_DATASET_H

#include <array>
#include <cstdint>
#include <mutex>

#include "../../../src/core/board/board.h"
#include "../../../src/eval/model.h"
#include "reader.h"

class Dataset {
  public:
    void Load(Reader& reader);
    std::vector<Game> GetBatch(size_t batch_size);
  private:
    std::vector<Game> elements_{};
    std::mutex mutex_;
};

#endif  // QUIRKY_TOOLS_EVAL_TUNER_STORAGE_DATASET_H
