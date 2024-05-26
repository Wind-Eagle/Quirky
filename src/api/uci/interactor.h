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

struct UciInitCommand {};
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
struct UciUnparsedCommand {
    std::string parse_error;
};

using uci_command_t =
    std::variant<UciInitCommand,UciReadyCommand, UciNewGameCommand, UciSetOptionCommand, UciPositionCommand,
                 UciGoCommand, UciStopCommand, UciQuitCommand, UciUnparsedCommand>;

struct UciInitResponse {};
struct UciReadyResponse {};
struct UciGoResponse {};
struct UciEmptyResponse {};
struct UciErrorResponse {
    std::string error_message;
    std::optional<QuirkyError> fatal_error;
};

using uci_response_t =
    std::variant<UciInitResponse,UciReadyResponse, UciGoResponse, UciEmptyResponse, UciErrorResponse>;

struct UciContext {
    q_search::Position position;
    std::vector<q_core::Move> moves;
    q_search::SearchLauncher launcher;
};

class UciInteractor {
  public:
    UciInteractor();
    uci_response_t ProcessUciCommand(const uci_command_t& command);
    bool ShouldStop() const;
  private:
    UciContext context_;
    bool should_stop_;
};

}  // namespace q_api

#endif  // QUIRKY_SRC_API_UCI_INTERACTOR_H
