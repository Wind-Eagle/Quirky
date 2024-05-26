#include "parser.h"
#include <string_view>

#include "interactor.h"
#include "../../util/string.h"

namespace q_api {

constexpr std::string_view STARTPOS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

uci_command_t ParseUciCommand(const std::string_view& command) {
    const std::vector<std::string> args = q_util::SplitString(command);
    const std::string_view command_name = args[0];
    if (command_name == "uci") {
        return UciInitCommand{};
    } else if (command_name == "isready") {
        return UciReadyCommand{};
    } else if (command_name == "ucinewgame") {
        return UciNewGameCommand{};
    } else if (command_name == "setoption") {
        return UciSetOptionCommand{};
    } else if (command_name == "position") {
        UciPositionCommand command;
        const auto move_word_pos = std::find(args.begin(), args.end(), "moves");
        std::string fen = q_util::ConcatenateStrings(args.begin() + 1, move_word_pos);
        if (fen == "startpos") {
            fen = STARTPOS_FEN;
        }
        command.fen = fen;
        if (move_word_pos == args.end()) {
            command.moves = std::nullopt;
        } else {
            command.moves = std::vector<std::string>{};
            for (auto i = move_word_pos + 1; i != args.end(); i++) {
                command.moves->push_back(*i);
            }
        }
        return command;
    } else if (command_name == "go") {
        UciGoCommand command;
        return command;
    } else if (command_name == "stop") {
        return UciStopCommand{};
    } else if (command_name == "quit") {
        return UciQuitCommand{};
    }
    return UciUnparsedCommand{.parse_error = "Unknown UCI command"};
}

}  // namespace q_api
