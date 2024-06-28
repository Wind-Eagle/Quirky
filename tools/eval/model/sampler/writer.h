#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H

#include "calcer.h"
#include "reader.h"
#include "weighter.h"

void WriteBoardsToCSV(BoardSetWithFeatures&& board_set_with_features, std::ofstream& out);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
