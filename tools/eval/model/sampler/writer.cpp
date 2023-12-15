#include "writer.h"

#include <fstream>
#include "calcer.h"

void WriteBoardsToCSV(BoardSetWithFeatures&& board_set_with_features, std::string_view path) {
    std::ofstream out(path.data());
    BoardSetWithFeatures boards_set = std::move(board_set_with_features);
    out << "stage,move_count,game_index";
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        out << ",f" << i;
    }
    for (size_t i = 0; i < PSQ_FEATURES_COUNT; i++) {
        out << "," << i;
    }
    out << '\n';
    for (const auto& board: boards_set.boards) {
        out << static_cast<int>(board.feature_storage_with_info.stage) << "," << static_cast<int>(board.board.move_count) << "," << board.feature_storage_with_info.game_index;
        for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
            out << "," << static_cast<int>(board.feature_storage_with_info.feature_storage[i]);
        }
        for (size_t i = 0; i < PSQ_FEATURES_COUNT; i++) {
            out << "," << static_cast<int>(board.feature_storage_with_info.psq_storage[i]);
        }
        out << '\n';
    }
}
