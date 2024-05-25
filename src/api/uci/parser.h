#ifndef QUIRKY_SRC_API_UCI_PARSER_H
#define QUIRKY_SRC_API_UCI_PARSER_H

#include "interactor.h"

namespace q_api {

std::shared_ptr<UciCommand> ParseUciCommand(std::string_view command);

}  // namespace q_api

#endif  // QUIRKY_SRC_API_UCI_PARSER_H
