#ifndef QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
#define QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H

#include "../../core/moves/movegen.h"
#include "position.h"

namespace q_search {

class MovePicker {
    public:
        explicit MovePicker(const Position& position);
        enum class Stage: uint8_t {
            Start = 0,
            Capture = 1,
            Promotion = 2,
            Simple = 3,
            End = 4
        };
        q_core::Move GetNextMove();
        Stage GetStage() const;
    private:
        void GetNewMoves();
        const Position& position_;
        q_core::MoveList list_;
        size_t pos_ = 0;
        Stage stage_ = Stage::Start;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
