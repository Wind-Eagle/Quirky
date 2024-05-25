#ifndef QUIRKY_SRC_API_UCI_INTERACTOR_H
#define QUIRKY_SRC_API_UCI_INTERACTOR_H

#include "../../search/control/time.h"
#include "../../search/searcher/searcher.h"
#include "../../search/searcher/launcher.h"

#include <optional>
#include <vector>

namespace q_api {

enum class OptionType : uint8_t {
    HashTableSize = 0
};

struct UciCommand{};

struct UciReadyCommand : public UciCommand {};
struct UciNewGameCommand : public UciCommand {};
struct UciSetOptionCommand : public UciCommand {
    OptionType type;
    std::string value;
};
struct UciPositionCommand : public UciCommand {
    std::string fen;
    std::optional<std::vector<std::string>> moves;
};
struct UciGoCommand : public UciCommand {
    q_search::TimeControl time_control;
    uint8_t max_depth;
};
struct UciStopCommand : public UciCommand {};
struct UciQuitCommand : public UciCommand {};

struct UciResponse{};

struct UciReadyResponse : public UciResponse{};
struct UciNewGameResponse : public UciResponse {};
struct UciSetOptionResponse : public UciResponse {};
struct UciPositionResponse : public UciCommand {};
struct UciGoResponse : public UciCommand {};
struct UciStopResponse : public UciCommand {};
struct UciQuitResponse : public UciCommand {};

class UciInteractor {
    public:
        std::shared_ptr<UciResponse> ProcessCommand(std::shared_ptr<UciCommand> command);
        bool ShouldStop() const;
    private:
        q_search::Position position_;
        q_search::SearchLauncher launcher_;
        bool should_stop_;
};

}  // namespace q_api

#endif  // QUIRKY_SRC_API_UCI_INTERACTOR_H
