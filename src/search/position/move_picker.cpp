#include "move_picker.h"

#include <algorithm>

namespace q_search {

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

MovePicker::Stage GetNextStage(MovePicker::Stage stage) {
    return static_cast<MovePicker::Stage>(static_cast<uint8_t>(stage) + 1);
}

MovePicker::MovePicker(const Position& position, const q_core::Move tt_move, const std::array<q_core::Move, KILLER_MOVES_COUNT>& killer_moves)
    : position_(position), tt_move_(tt_move), killer_moves_(killer_moves) {}

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
            case Stage::KillerMoves: {
                for (size_t i = 0; i < KILLER_MOVES_COUNT; i++) {
                    if (!q_core::IsMoveNull(killer_moves_[i]) && q_core::IsMovePseudolegal(position_.board, killer_moves_[i])) {
                        list_.moves[list_.size] = killer_moves_[i];
                        list_.size++;
                    }
                }
                break;
            }
            case Stage::Simple: {
                const size_t list_old_size = list_.size;
                q_core::GenerateAllSimpleMoves(position_.board, list_);
                for (size_t i = list_old_size; i < list_.size; i++) {
                    bool is_killer_move = false;
                    for (size_t j = 0; j < KILLER_MOVES_COUNT; j++) {
                        if (q_core::GetCompressedMove(list_.moves[i]) == q_core::GetCompressedMove(killer_moves_[j])) {
                            is_killer_move = true;
                            break;
                        }
                    }
                    if (is_killer_move) {
                        std::swap(list_.moves[i], list_.moves[list_.size - 1]);
                        list_.size--;
                    }
                }
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

QuiescenseMovePicker::Stage GetNextStage(QuiescenseMovePicker::Stage stage) {
    return static_cast<QuiescenseMovePicker::Stage>(static_cast<uint8_t>(stage) + 1);
}

QuiescenseMovePicker::QuiescenseMovePicker(const Position& position)
    : position_(position) {}

q_core::Move QuiescenseMovePicker::GetNextMove() {
    GetNewMoves();
    if (Q_UNLIKELY(stage_ == Stage::End)) {
        return q_core::NULL_MOVE;
    }
    return list_.moves[pos_++];
}

void QuiescenseMovePicker::GetNewMoves() {
    while (pos_ == list_.size) {
        if (stage_ != Stage::End) {
            stage_ = GetNextStage(stage_);
        }
        switch (stage_) {
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
            case Stage::End: {
                return;
            }
            default: {
                Q_UNREACHABLE();
            }
        }
    }
}

QuiescenseMovePicker::Stage QuiescenseMovePicker::GetStage() const { return stage_; }

}  // namespace q_search
