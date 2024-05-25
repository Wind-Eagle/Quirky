#include "logger.h"
#include "interactor.h"

namespace q_api {

#define LOG_UCI_RESPONSE_FUNCTION(type)  \
    void Log##type(std::shared_ptr<type> response)

LOG_UCI_RESPONSE_FUNCTION(UciReadyResponse) {

}

LOG_UCI_RESPONSE_FUNCTION(UciNewGameResponse) {

}

LOG_UCI_RESPONSE_FUNCTION(UciSetOptionResponse) {

}

LOG_UCI_RESPONSE_FUNCTION(UciPositionResponse) {

}

LOG_UCI_RESPONSE_FUNCTION(UciGoResponse) {

}

LOG_UCI_RESPONSE_FUNCTION(UciStopResponse) {

}

LOG_UCI_RESPONSE_FUNCTION(UciQuitResponse) {

}

#define LOG_UCI_RESPONSE(type, response_interface)  \
    if (auto response = std::static_pointer_cast<type>(response_interface)) {  \
        Log##type(response);  \
        return;  \
    }

void LogUciResponse(std::shared_ptr<UciResponse> response_interface) {
    LOG_UCI_RESPONSE(UciReadyResponse, response_interface);
    q_util::PrintError("Unknown uci response type");
    q_util::ExitWithError(QuirkyError::UnexpectedValue);
}

}  // namespace q_api