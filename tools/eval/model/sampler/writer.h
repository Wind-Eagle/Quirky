#ifndef QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
#define QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H

#include "reader.h"
#include "calcer.h"
#include "filter.h"

void WriteBoardsToCSV(BoardSetWithFeatures&& board_set_with_features, std::string_view path);

#endif  // QUIRKY_TOOLS_EVAL_MODEL_SAMPLER_WRITER_H
