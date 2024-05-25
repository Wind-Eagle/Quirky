#include "interactor.h"
#include <memory>

namespace q_api {

void PrintGreeting() {
    q_util::Print("id name Quirky");
    q_util::Print("id author Wind_Eagle");
    q_util::Print("uciok");
}

void PrintReadiness() {
    q_util::Print("readyok");
}

std::shared_ptr<UciResponse> UciInteractor::ProcessCommand(std::shared_ptr<UciCommand> command) {
    return std::make_shared<UciReadyResponse>();
}

bool UciInteractor::ShouldStop() const {
    return should_stop_;
}

}  // namespace q_api
