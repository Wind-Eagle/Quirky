#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H

#include "reader.h"

struct OutputSources {
    std::ofstream preliminary_train_out;
    std::ofstream preliminary_test_out;
    std::vector<std::ofstream> train_outs;
    std::ofstream test_out;

    float test_ratio;
    float preliminary_ratio;
    size_t chunks_count;
};

void WriteBoardsToCSV(const PositionSet& position_set, OutputSources& output_sources);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
