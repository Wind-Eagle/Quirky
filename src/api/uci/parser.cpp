#include "parser.h"
#include "interactor.h"

namespace q_api {

std::shared_ptr<UciCommand> ParseUciCommand(std::string_view command) {
    return std::make_shared<UciReadyCommand>();
}

}  // namespace q_api
