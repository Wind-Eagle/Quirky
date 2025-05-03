#include "protocol.h"

#include "interactor.h"
#include "logger.h"
#include "parser.h"
#include "util/io.h"

namespace q_api {

void StartUciProtocol() {
    UciInteractor interactor;
    LogStart();
    do {
        const auto line = q_util::ReadLine();
        if (!line) {
            break;
        }
        const auto command = ParseUciCommand(*line);
        const auto response = interactor.ProcessUciCommand(command);
        LogUciResponse(response);
    } while (!interactor.ShouldStop());
}

}  // namespace q_api
