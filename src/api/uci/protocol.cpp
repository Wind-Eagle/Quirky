#include "protocol.h"
#include <memory>

#include "interactor.h"
#include "logger.h"
#include "parser.h"

namespace q_api {

constexpr std::string_view STARTPOS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void StartUciProtocol() {
    UciInteractor interactor;
    do {
        auto command = ParseUciCommand("");
        auto response = interactor.ProcessUciCommand(command);
        LogUciResponse(response);
    } while (!interactor.ShouldStop());
}

}  // namespace q_api
