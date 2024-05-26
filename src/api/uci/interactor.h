#ifndef QUIRKY_SRC_API_UCI_INTERACTOR_H
#define QUIRKY_SRC_API_UCI_INTERACTOR_H

#include <optional>
#include <variant>
#include <vector>

#include "../../search/control/time.h"
#include "../../search/searcher/launcher.h"
#include "../../search/searcher/searcher.h"

namespace q_api {

enum class OptionType : uint8_t { HashTableSize = 0 };

struct UciReadyCommand {};
struct UciNewGameCommand {};
struct UciSetOptionCommand {
    OptionType type;
    std::string value;
};
struct UciPositionCommand {
    std::string fen;
    std::optional<std::vector<std::string>> moves;
};
struct UciGoCommand {
    q_search::TimeControl time_control;
    uint8_t max_depth;
};
struct UciStopCommand {};
struct UciQuitCommand {};

using uci_command_t =
    std::variant<UciReadyCommand, UciNewGameCommand, UciSetOptionCommand, UciPositionCommand,
                 UciGoCommand, UciStopCommand, UciQuitCommand>;

struct UciReadyResponse {};
struct UciNewGameResponse {};
struct UciSetOptionResponse {};
struct UciPositionResponse {};
struct UciGoResponse {};
struct UciStopResponse {};
struct UciQuitResponse {};

using uci_response_t =
    std::variant<UciReadyResponse, UciNewGameResponse, UciSetOptionResponse, UciPositionResponse,
                 UciGoResponse, UciStopResponse, UciQuitResponse>;

class UciInteractor {
  public:
    uci_response_t ProcessUciCommand(const uci_command_t& command);
    bool ShouldStop() const;

  private:
    q_search::Position position_;
    q_search::SearchLauncher launcher_;
    bool should_stop_;
};

}  // namespace q_api

#endif  // QUIRKY_SRC_API_UCI_INTERACTOR_H
