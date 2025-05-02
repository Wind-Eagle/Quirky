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
        if (args.size() != 5) {
            return UciUnparsedCommand{.parse_error = "Invalid number of arguments"};
        }
        if (args[1] != "key" && args[3] != "value") {
            return UciUnparsedCommand{.parse_error = "Expected key and value"};
        }
        if (args[2] == "Hash") {
            return UciSetOptionCommand{.type = OptionType::HashTableSize, .value = args[4]};
        }
        return UciUnparsedCommand{.parse_error = "No such option"};
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
        if (args.size() == 1) {
            return command;
        }
        if (args[1] == "infinite") {
            if (args.size() > 2) {
                return UciUnparsedCommand{.parse_error = "Infinite argument must be used alone"};
            }
        } else if (args[1] == "movetime") {
            if (args.size() == 2) {
                return UciUnparsedCommand{.parse_error =
                                              "Movetime argument must be used with amount of "
                                              "time as a next argument"};
            }
            if (!q_util::IsStringNonNegativeNumber(args[2])) {
                return UciUnparsedCommand{.parse_error = "Expected valid argument as time"};
            }
            if (args.size() != 3) {
                return UciUnparsedCommand{
                    .parse_error = "Movetime must be followed by precisely one argument"};
            }
            command.time_control = q_search::FixedTimeControl{
                .time = static_cast<q_search::time_t>(std::stoll(args[2]))};
        } else if (args[1] == "depth") {
            if (args.size() == 2) {
                return UciUnparsedCommand{.parse_error =
                                              "Movetime argument must be used with amount of "
                                              "time as a next argument"};
            }
            if (!q_util::IsStringNonNegativeNumber(args[2])) {
                return UciUnparsedCommand{.parse_error = "Expected valid argument as depth"};
            }
            uint64_t depth_int = std::stoll(args[2]);
            if (depth_int > q_search::Searcher::MAX_DEPTH) {
                return UciUnparsedCommand{.parse_error =
                                              "Depth should be not more than " +
                                              std::to_string(q_search::Searcher::MAX_DEPTH)};
            }
            if (args.size() != 3) {
                return UciUnparsedCommand{.parse_error =
                                              "Depth must be followed by precisely one argument"};
            }
            command.max_depth = depth_int;
        } else {
            q_search::GameTimeControl time_control{};
            for (size_t i = 1; i < args.size(); i += 2) {
                if (!q_util::IsStringNonNegativeNumber(args[i + 1])) {
                    return UciUnparsedCommand{.parse_error = "Expected valid argument as time"};
                }
                uint64_t arg = std::stoll(args[i + 1]);
                if (args[i] == "wtime") {
                    time_control.white_time.time = arg;
                } else if (args[i] == "btime") {
                    time_control.black_time.time = arg;
                } else if (args[i] == "winc") {
                    time_control.white_time.increment = arg;
                } else if (args[i] == "binc") {
                    time_control.black_time.increment = arg;
                } else if (args[i] == "movestogo") {
                    time_control.moves_to_go = arg;
                } else {
                    return UciUnparsedCommand{.parse_error = "Unsupported argument: " + args[i]};
                }
            }
            command.time_control = time_control;
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
