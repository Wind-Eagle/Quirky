#include "writer.h"

#include <fstream>

#include "calcer.h"
#include "reader.h"

std::string GetTarget(Result result) {
    switch (result) {
        case Result::WhiteWins: {
            return "1";
        }
        case Result::Draw: {
            return "0.5";
        }
        case Result::BlackWins: {
            return "0";
        }
        default: {
            q_util::ExitWithError(QuirkyError::UnexpectedValue);
        }
    }
    return "0.5";
}

void WriteBoardsToCSV(BoardSetWithFeatures&& board_set_with_features, std::ofstream& out) {
    BoardSetWithFeatures boards_set = std::move(board_set_with_features);
    out << "stage,move_count,game_index,weight";
    for (size_t i = 0; i < q_core::NUMBER_OF_PIECES; i++) {
        out << ",p" << i;
    }
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        out << ",f" << i;
    }
    for (size_t i = 0; i < PSQ_FEATURES_COUNT; i++) {
        out << "," << i;
    }
    out << ",target\n";
    for (const auto& board : boards_set.boards) {
        out << static_cast<int>(board.feature_storage_with_info.stage) << ","
            << static_cast<int>(board.board.move_count) << ","
            << board.feature_storage_with_info.game_index << ","
            << std::to_string(board.feature_storage_with_info.weight);
        for (size_t i = 0; i < q_core::NUMBER_OF_PIECES; i++) {
            out << "," << static_cast<int>(board.feature_storage_with_info.piece_count_storage[i]);
        }
        for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
            out << "," << static_cast<int>(board.feature_storage_with_info.feature_storage[i]);
        }
        for (size_t i = 0; i < PSQ_FEATURES_COUNT; i++) {
            out << "," << static_cast<int>(board.feature_storage_with_info.psq_storage[i]);
        }
        out << "," << GetTarget(board.feature_storage_with_info.result) << '\n';
    }
}
