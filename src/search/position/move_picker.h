#ifndef QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
#define QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H

#include "../../core/moves/movegen.h"
#include "position.h"

namespace q_search {

class MovePicker {
    public:
        static constexpr uint8_t KILLER_MOVES_COUNT = 2;
        MovePicker(const Position& position, q_core::Move tt_move, const std::array<q_core::Move, KILLER_MOVES_COUNT>& killer_moves);
        enum class Stage: uint8_t {
            Start = 0,
            TTMove = 1,
            Capture = 2,
            Promotion = 3,
            KillerMoves = 4,
            Simple = 5,
            End = 6
        };
        q_core::Move GetNextMove();
        Stage GetStage() const;
    private:
        void GetNewMoves();
        const Position& position_;
        q_core::MoveList list_;
        q_core::Move tt_move_;
        std::array<q_core::Move, KILLER_MOVES_COUNT> killer_moves_;
        size_t pos_ = 0;
        Stage stage_ = Stage::Start;
};

class QuiescenseMovePicker {
    public:
        explicit QuiescenseMovePicker(const Position& position);
        enum class Stage: uint8_t {
            Start = 0,
            Capture = 1,
            Promotion = 2,
            End = 3
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
