#include "interactor.h"
#include <memory>

namespace q_api {

#define PROCESS_UCI_COMMAND_FUNCTION(type)  \
uci_response_t Process##type(const type& command)

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

#define PROCESS_UCI_COMMAND(type, command)  \
    if constexpr (std::is_same_v<std::decay_t<decltype(command)>, type>) {  \
        return Process##type(command);  \
    }

uci_response_t UciInteractor::ProcessUciCommand(const uci_command_t& command) {
    std::visit([](auto&& command)
        {
            PROCESS_UCI_COMMAND(UciReadyCommand, command);
            PROCESS_UCI_COMMAND(UciNewGameCommand, command);
            PROCESS_UCI_COMMAND(UciSetOptionCommand, command);
            PROCESS_UCI_COMMAND(UciPositionCommand, command);
            PROCESS_UCI_COMMAND(UciGoCommand, command);
            PROCESS_UCI_COMMAND(UciStopCommand, command);
            PROCESS_UCI_COMMAND(UciQuitCommand, command);
        }, command);
}

bool UciInteractor::ShouldStop() const {
    return should_stop_;
}

}  // namespace q_api
