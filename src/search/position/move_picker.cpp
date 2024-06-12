#include "move_picker.h"

#include <algorithm>

namespace q_search {

MovePicker::Stage GetNextStage(MovePicker::Stage stage) {
    return static_cast<MovePicker::Stage>(static_cast<uint8_t>(stage) + 1);
}

MovePicker::MovePicker(const Position& position, const q_core::Move tt_move)
    : position_(position), tt_move_(tt_move) {}

q_core::Move MovePicker::GetNextMove() {
    GetNewMoves();
    if (Q_UNLIKELY(stage_ == Stage::End)) {
        return q_core::NULL_MOVE;
    }
    return list_.moves[pos_++];
}

static constexpr uint8_t CAPTURE_VICTIM_COST[q_core::NUMBER_OF_CELLS] = {0, 8,  16, 24, 30, 36, 0,
                                                                         8, 16, 24, 30, 36, 0};
static constexpr uint8_t CAPTURE_ATTACKER_COST[q_core::NUMBER_OF_CELLS] = {0, 5, 4, 3, 2, 1, 6,
                                                                           5, 4, 3, 2, 1, 6};

static void SortByMVVLVA(const q_core::Board& board, q_core::Move* moves, const size_t count) {
    for (size_t i = 0; i < count; ++i) {
        q_core::Move& move = moves[i];
        move.info = CAPTURE_VICTIM_COST[board.cells[move.dst]] +
                    CAPTURE_ATTACKER_COST[board.cells[move.src]];
    }
    std::sort(moves, moves + count,
              [](const q_core::Move lhs, const q_core::Move rhs) { return lhs.info > rhs.info; });
}

void MovePicker::GetNewMoves() {
    while (pos_ == list_.size) {
        if (stage_ != Stage::End) {
            stage_ = GetNextStage(stage_);
        }
        switch (stage_) {
            case Stage::TTMove: {
                if (!q_core::IsMoveNull(tt_move_)) {
                    list_.moves[list_.size] = tt_move_;
                    list_.size++;
                }
                break;
            }
            case Stage::Capture: {
                const size_t list_old_size = list_.size;
                q_core::GenerateAllCaptures(position_.board, list_);
                SortByMVVLVA(position_.board, list_.moves + list_old_size,
                             list_.size - list_old_size);
                break;
            }
            case Stage::Promotion: {
                q_core::GenerateAllPromotions(position_.board, list_);
                break;
            }
            case Stage::Simple: {
                q_core::GenerateAllSimpleMoves(position_.board, list_);
                break;
            }
            case Stage::End: {
                return;
            }
            default: {
                Q_UNREACHABLE();
            }
        }
    }
}

MovePicker::Stage MovePicker::GetStage() const { return stage_; }

}  // namespace q_search
