#include "logger.h"
#include "interactor.h"

namespace q_api {

#define LOG_UCI_RESPONSE_FUNCTION(type)  \
void Log##type(const type& response)

LOG_UCI_RESPONSE_FUNCTION(UciReadyCommand) {

}

LOG_UCI_RESPONSE_FUNCTION(UciNewGameCommand) {

}

LOG_UCI_RESPONSE_FUNCTION(UciSetOptionCommand) {

}

LOG_UCI_RESPONSE_FUNCTION(UciPositionCommand) {

}

LOG_UCI_RESPONSE_FUNCTION(UciGoCommand) {

}

LOG_UCI_RESPONSE_FUNCTION(UciStopCommand) {

}

LOG_UCI_RESPONSE_FUNCTION(UciQuitCommand) {

}

#define LOG_UCI_RESPONSE(type, response)  \
    if constexpr (std::is_same_v<std::decay_t<decltype(response)>, type>) {  \
        return Log##type(command);  \
    }

void LogUciResponse(const uci_command_t& response) {
    std::visit([](auto&& command)
        {
            LOG_UCI_RESPONSE(UciReadyResponse, command);
            LOG_UCI_RESPONSE(UciNewGameResponse, command);
            LOG_UCI_RESPONSE(UciSetOptionResponse, command);
            LOG_UCI_RESPONSE(UciPositionResponse, command);
            LOG_UCI_RESPONSE(UciGoResponse, command);
            LOG_UCI_RESPONSE(UciStopResponse, command);
            LOG_UCI_RESPONSE(UciQuitResponse, command);
        }, response);
}

}  // namespace q_api