#include "writer.h"

#include <fstream>
#include "calcer.h"

void WriteBoardsToCSV(BoardSetWithFeatures&& board_set_with_features, std::string_view path) {
    std::ofstream out(path.data());
    BoardSetWithFeatures boards_set = std::move(board_set_with_features);
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        out << "f" << i << ",";
    }
    for (size_t i = 0; i < PSQ_FEATURES_COUNT; i++) {
        out << i << ",";
    }
    out << "stage,move_count,game_index" << '\n';
    for (const auto& board: boards_set.boards) {
        for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
            out << static_cast<int>(board.feature_storage_with_info.feature_storage[i]) << ",";
        }
        for (size_t i = 0; i < PSQ_FEATURES_COUNT; i++) {
            out << static_cast<int>(board.feature_storage_with_info.psq_storage[i]) << ",";
        }
        out << static_cast<int>(board.feature_storage_with_info.stage) << "," << board.board.move_count << " " << board.feature_storage_with_info.game_index << '\n';
    }
}
