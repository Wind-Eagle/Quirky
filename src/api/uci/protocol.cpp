#include "protocol.h"

#include <memory>

#include "interactor.h"
#include "logger.h"
#include "parser.h"

namespace q_api {

void StartUciProtocol() {
    UciInteractor interactor;
    LogStart();
    do {
        const std::string line = q_util::ReadLine();
        const auto command = ParseUciCommand(line);
        const auto response = interactor.ProcessUciCommand(command);
        LogUciResponse(response);
    } while (!interactor.ShouldStop());
}

}  // namespace q_api
