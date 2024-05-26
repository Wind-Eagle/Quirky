#include "interactor.h"
#include <memory>

namespace q_api {

#define PROCESS_UCI_COMMAND_FUNCTION(type)  \
    std::shared_ptr<UciResponse> Process##type(std::shared_ptr<type> command)

PROCESS_UCI_COMMAND_FUNCTION(UciReadyCommand) {

}

PROCESS_UCI_COMMAND_FUNCTION(UciNewGameCommand) {

}

PROCESS_UCI_COMMAND_FUNCTION(UciSetOptionCommand) {

}

PROCESS_UCI_COMMAND_FUNCTION(UciPositionCommand) {

}

PROCESS_UCI_COMMAND_FUNCTION(UciGoCommand) {

}

PROCESS_UCI_COMMAND_FUNCTION(UciStopCommand) {

}

PROCESS_UCI_COMMAND_FUNCTION(UciQuitCommand) {

}

#define PROCESS_UCI_COMMAND(type, command_interface)  \
    if (auto command = std::static_pointer_cast<type>(command_interface)) {  \
        return Process##type(command);  \
    }

std::shared_ptr<UciResponse> UciInteractor::ProcessUciCommand(std::shared_ptr<UciCommand> command_interface) {
    PROCESS_UCI_COMMAND(UciReadyCommand, command_interface);
    q_util::PrintError("Unknown uci command type");
    q_util::ExitWithError(QuirkyError::UnexpectedValue);
}

bool UciInteractor::ShouldStop() const {
    return should_stop_;
}

}  // namespace q_api
