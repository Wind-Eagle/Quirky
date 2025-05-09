#include "logger.h"

#include <optional>

#include "interactor.h"
#include "util/io.h"

namespace q_api {

void LogStart() {
    q_util::Print("Hello! I'm Quirky, a chess engine. Use UCI protocol to communicate with me.");
}

void LogUciResponseInner(const UciInitResponse&) {
    q_util::Print("id name Quirky 1.0");
    q_util::Print("id author Wind-Eagle");
    q_util::Print("option name Hash type spin default 16 min 1 max 33554432");
    q_util::Print("uciok");
}

void LogUciResponseInner(const UciReadyResponse&) { q_util::Print("readyok"); }

void LogUciResponseInner(const UciEmptyResponse&) {}

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