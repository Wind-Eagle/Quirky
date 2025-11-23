#include "interactor.h"

namespace q_api {

constexpr std::string_view STARTPOS_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

uci_response_t ProcessUciCommandInner(UciContext&, const UciInitCommand&) {
    return UciInitResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext&, const UciReadyCommand&) {
    return UciReadyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciNewGameCommand&) {
    context.launcher.NewGame();
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciSetOptionCommand& command) {
    switch (command.type) {
        case OptionType::HashTableSize: {
            context.launcher.ChangeTTSize(std::stoll((command.value)));
        }
        case OptionType::PVCount: {
            context.launcher.ChangePVCount(std::stoll((command.value)));
        }
    }
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
        q_core::Board check_board = context.position.board;
        for (const auto& move_str : *command.moves) {
            if (!q_core::IsStringMoveWellFormated(check_board, move_str)) {
                return UciErrorResponse{.error_message = "Invalid move string",
                                        .fatal_error = std::nullopt};
            }
            q_core::Move move = q_core::TranslateStringToMove(check_board, move_str);
            context.moves.push_back(move);
            q_core::MakeMoveInfo make_move_info;
            q_core::MakeMove(check_board, move, make_move_info);
        }
    }
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciGoCommand& command) {
    context.launcher.Start(context.position, context.moves, command.time_control,
                           command.max_depth);
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciStopCommand&) {
    context.launcher.Join();
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext& context, const UciQuitCommand&) {
    context.should_stop = true;
    return UciEmptyResponse{};
}

uci_response_t ProcessUciCommandInner(UciContext&, const UciUnparsedCommand& command) {
    return UciErrorResponse{.error_message = command.parse_error, .fatal_error = std::nullopt};
}

uci_response_t UciInteractor::ProcessUciCommand(const uci_command_t& command) {
    return std::visit(
        [this](const auto& command) { return ProcessUciCommandInner(context_, command); }, command);
}

UciInteractor::UciInteractor() {
    context_.position.MakeFromFEN(STARTPOS_FEN);
    context_.should_stop = false;
}

bool UciInteractor::ShouldStop() const { return context_.should_stop; }

}  // namespace q_api
