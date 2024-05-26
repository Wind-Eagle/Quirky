#ifndef QUIRKY_SRC_API_UCI_PARSER_H
#define QUIRKY_SRC_API_UCI_PARSER_H

#include "interactor.h"

namespace q_api {

uci_command_t ParseUciCommand(const std::string_view& command);
    
}  // namespace q_api

#endif  // QUIRKY_SRC_API_UCI_PARSER_H
