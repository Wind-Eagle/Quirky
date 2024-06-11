#ifndef QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
#define QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H

#include "../../core/moves/movegen.h"
#include "position.h"

namespace q_search {

class MovePicker {
    public:
        MovePicker(const Position& position, q_core::Move tt_move);
        enum class Stage: uint8_t {
            Start = 0,
            TTMove = 1,
            Capture = 2,
            Promotion = 3,
            Simple = 4,
            End = 5
        };
        q_core::Move GetNextMove();
        Stage GetStage() const;
    private:
        void GetNewMoves();
        const Position& position_;
        q_core::MoveList list_;
        q_core::Move tt_move_;
        size_t pos_ = 0;
        Stage stage_ = Stage::Start;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
