#include "filter.h"
#include "calcer.h"

BoardSetWithFeatures FilterBoards(BoardSetWithFeatures&& board_set_with_features) {
    BoardSetWithFeatures boards_set = std::move(board_set_with_features);
    boards_set.boards.erase(std::remove_if(boards_set.boards.begin(), boards_set.boards.end(), [](BoardWithFeatures& board) {
        return false;
    }), boards_set.boards.end());
    return boards_set;
}