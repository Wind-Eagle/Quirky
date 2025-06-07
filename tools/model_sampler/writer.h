#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H

#include "reader.h"

void WriteBoardsToCSV(const PositionSet& game_set, std::ofstream& out);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
