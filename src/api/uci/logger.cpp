#include "logger.h"

#include <optional>

#include "interactor.h"

namespace q_api {

void LogStart() {
    q_util::Print("Hello! I'm Quirky, a chess engine. Use UCI protocol to communicate with me.");
}

void LogUciResponseInner(const UciInitResponse& response) {
    q_util::Print("id name Quirky");
    q_util::Print("id author Wind_Eagle");
    q_util::Print("uciok");
}

void LogUciResponseInner(const UciReadyResponse& response) { q_util::Print("readyok"); }

void LogUciResponseInner(const UciEmptyResponse& response) {}

void LogUciResponseInner(const UciErrorResponse& response) {
    q_util::PrintError(response.error_message);
    if (response.fatal_error != std::nullopt) {
        q_util::ExitWithError(*response.fatal_error);
    }
}

void LogUciResponse(const uci_response_t& response) {
    std::visit([](const auto& response) { LogUciResponseInner(response); }, response);
}

}  // namespace q_api