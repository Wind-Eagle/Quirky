#include "weighter.h"

#include <cmath>

#include "calcer.h"

BoardSetWithFeatures AddWeightsToBoards(BoardSetWithFeatures&& board_set_with_features) {
    BoardSetWithFeatures boards_set = std::move(board_set_with_features);
    size_t total_boards_count = boards_set.boards.size();
    for (size_t i = 0; i < total_boards_count; i++) {
        auto& board = boards_set.boards[i];
        float w = std::exp(-static_cast<double>(board.feature_storage_with_info.last_move_number -
                                                board.board.move_count) /
                           32.0);
        board.feature_storage_with_info.weight = w;
    }
    return boards_set;
}
