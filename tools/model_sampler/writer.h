#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H

#include "reader.h"

void WriteBoardsToCSV(const PositionSet& position_set, std::ofstream& train_out, std::ofstream& test_out, float ratio);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
