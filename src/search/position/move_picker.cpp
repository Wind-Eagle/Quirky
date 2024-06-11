#include "move_picker.h"

namespace q_search {

MovePicker::Stage GetNextStage(MovePicker::Stage stage) {
    return static_cast<MovePicker::Stage>(static_cast<uint8_t>(stage) + 1);
}

MovePicker::MovePicker(const Position& position, const q_core::Move tt_move) : position_(position), tt_move_(tt_move) {}

q_core::Move MovePicker::GetNextMove() {
    GetNewMoves();
    if (Q_UNLIKELY(stage_ == Stage::End)) {
        return q_core::NULL_MOVE;
    }
    return list_.moves[pos_++];
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
                q_core::GenerateAllCaptures(position_.board, list_);
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

MovePicker::Stage MovePicker::GetStage() const {
    return stage_;
}

}  // namespace q_search
