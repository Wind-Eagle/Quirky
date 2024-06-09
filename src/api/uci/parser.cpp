#include "parser.h"

#include <string_view>

#include "../../util/string.h"
#include "interactor.h"

namespace q_api {

constexpr std::string_view STARTPOS_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
        if (args.size() == 1) {
            return UciUnparsedCommand{.parse_error = "Expected arguments"};
        }
        if (args[1] != "fen" && args[1] != "startpos") {
            return UciUnparsedCommand{.parse_error = "Expected fen or startpos argument"};
        }
        UciPositionCommand command;
        const auto move_word_pos = std::find(args.begin(), args.end(), "moves");
        const std::string fen = args[1] == "startpos"
                                    ? static_cast<std::string>(STARTPOS_FEN)
                                    : q_util::ConcatenateStrings(args.begin() + 2, move_word_pos);
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
        command.time_control = q_search::InfiniteTimeControl{};
        command.max_depth = q_search::Searcher::MAX_DEPTH;
        for (size_t i = 1; i < args.size(); i += 2) {
            if (args[i] == "infinite") {
                if (i > 1) {
                    return UciUnparsedCommand{.parse_error =
                                                  "Infinite argument must be used alone"};
                }
            } else if (args[i] == "movetime") {
                if (i + 1 == args.size()) {
                    return UciUnparsedCommand{.parse_error =
                                                  "Movetime argument must be used with amount of "
                                                  "time as a next argument"};
                }
                if (!q_util::IsStringNonNegativeNumber(args[i + 1])) {
                    return UciUnparsedCommand{.parse_error = "Expected valid argument as time"};
                }
                command.time_control = q_search::FixedTimeControl{
                    .time = static_cast<q_search::time_t>(std::stoll(args[i + 1]))};
            } else if (args[i] == "depth") {
                if (i + 1 == args.size()) {
                    return UciUnparsedCommand{.parse_error =
                                                  "Movetime argument must be used with amount of "
                                                  "time as a next argument"};
                }
                if (!q_util::IsStringNonNegativeNumber(args[i + 1])) {
                    return UciUnparsedCommand{.parse_error = "Expected valid argument as depth"};
                }
                uint64_t depth_int = std::stoll(args[i + 1]);
                if (depth_int > q_search::Searcher::MAX_DEPTH) {
                    return UciUnparsedCommand{.parse_error =
                                                  "Depth should be not more than " +
                                                  std::to_string(q_search::Searcher::MAX_DEPTH)};
                }
                command.max_depth = std::stoll(args[i + 1]);
            } else {
                return UciUnparsedCommand{.parse_error = "Unsupported option"};
            }
        }
        return command;
    } else if (command_name == "stop") {
        return UciStopCommand{};
    } else if (command_name == "quit") {
        return UciQuitCommand{};
    }
    return UciUnparsedCommand{.parse_error = "Unknown UCI command"};
}

}  // namespace q_api
