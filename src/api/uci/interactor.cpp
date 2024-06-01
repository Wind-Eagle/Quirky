#include "interactor.h"
#include <memory>

namespace q_api {

constexpr std::string_view STARTPOS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

uci_response_t ProcessUciCommandInner(UciContext&, const UciInitCommand& command) {
    return UciInitResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext&, const UciReadyCommand& command) {
    return UciReadyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciNewGameCommand& command) {
    context.launcher.NewGame();
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext&, const UciSetOptionCommand& command) {
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciPositionCommand& command) {
    auto res = context.position.MakeFromFEN(command.fen);
    if (res != q_core::Board::FENParseStatus::Ok) {
        context.position.MakeFromFEN(STARTPOS_FEN);
        return UciErrorResponse{.error_message = "Invalid FEN", .fatal_error = std::nullopt};
    }
    context.moves.clear();
    if (command.moves != std::nullopt) {
        for (const auto& move_str : *command.moves) {
            if (!q_core::IsStringMoveWellFormated(context.position.board, move_str)) {
                return UciErrorResponse{.error_message = "Invalid move string", .fatal_error = std::nullopt};
            }
            context.moves.push_back(q_core::TranslateStringToMove(context.position.board, move_str));
        }
    }
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciGoCommand& command) {
    context.launcher.Start(context.position, context.moves, command.time_control, command.max_depth);
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciStopCommand& command) {
    context.launcher.Stop();
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext&, const UciQuitCommand& command) {
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext&, const UciUnparsedCommand& command) {
    return UciErrorResponse{.error_message = command.parse_error, .fatal_error = std::nullopt};
}

uci_response_t UciInteractor::ProcessUciCommand(const uci_command_t& command) {
    return std::visit([this](const auto& command)
        {
            return ProcessUciCommandInner(context_, command);
        }, command);
}

UciInteractor::UciInteractor() {
    context_.position.MakeFromFEN(STARTPOS_FEN);
    should_stop_ = false;
}

bool UciInteractor::ShouldStop() const {
    return should_stop_;
}

}  // namespace q_api
