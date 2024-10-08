#ifndef QUIRKY_TOOLS_EVAL_TUNER_PARSER_LEARNER_H
#define QUIRKY_TOOLS_EVAL_TUNER_PARSER_LEARNER_H

#include <memory>

#include "calcer.h"
#include "dataset.h"
#include "reader.h"

struct LearnerParams {
    std::shared_ptr<Dataset> dataset;
    std::string output_filename;
    std::string state_filename;
    size_t number_of_threads;
    size_t channel_size;
};

void TuneWeights(const LearnerParams& params);

#endif  // QUIRKY_TOOLS_EVAL_TUNER_PARSER_LEARNER_H
