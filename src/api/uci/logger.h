#ifndef QUIRKY_SRC_API_UCI_LOGGER_H
#define QUIRKY_SRC_API_UCI_LOGGER_H

#include "interactor.h"

namespace q_api {

void LogStart();
void LogUciResponse(const uci_response_t& response);

}  // namespace q_api

#endif  // QUIRKY_SRC_API_UCI_LOGGER_H
