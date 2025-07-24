#include "move_picker.h"

#include <algorithm>

#include "core/moves/movegen.h"
#include "position.h"

namespace q_search {

KillerMoves::KillerMoves() {
    for (size_t i = 0; i < COUNT; i++) {
        moves_[i] = q_core::NULL_MOVE;
    }
}

void KillerMoves::Add(const q_core::Move move) {
    for (size_t j = COUNT - 1; j >= 1; j--) {
        moves_[j] = moves_[j - 1];
    }
    moves_[0] = move;
}

q_core::Move KillerMoves::GetMove(const uint8_t index) const { return moves_[index]; }

HistoryTable::HistoryTable() {
    for (q_core::cell_t i = 0; i < q_core::NUMBER_OF_CELLS; i++) {
        for (q_core::coord_t j = 0; j < q_core::BOARD_SIZE; j++) {
            table_[i][j] = 0;
        }
    }
}

void HistoryTable::Update(const q_core::cell_t cell, const q_core::Move move, const depth_t depth) {
    table_[cell][move.dst] += depth * depth;
}

uint64_t HistoryTable::GetScore(const q_core::cell_t cell, const q_core::Move move) const {
    return table_[cell][move.dst];
}

static constexpr uint8_t CAPTURE_VICTIM_COST[q_core::NUMBER_OF_CELLS] = {0, 8,  16, 24, 30, 36, 0,
                                                                         8, 16, 24, 30, 36, 0};
static constexpr uint8_t CAPTURE_ATTACKER_COST[q_core::NUMBER_OF_CELLS] = {0, 5, 4, 3, 2, 1, 6,
                                                                           5, 4, 3, 2, 1, 6};

inline static void AnnotateByMVVLVA(const q_core::Board& board, q_core::Move* moves,
                                    const size_t count) {
    for (size_t i = 0; i < count; ++i) {
        q_core::Move& move = moves[i];
        move.info = CAPTURE_VICTIM_COST[board.cells[move.dst]] +
                    CAPTURE_ATTACKER_COST[board.cells[move.src]];
    }
}

inline static void SortByMVVLVA(const q_core::Board& board, q_core::Move* moves,
                                const size_t count) {
    for (size_t i = 0; i < count; ++i) {
        q_core::Move& move = moves[i];
        move.info = CAPTURE_VICTIM_COST[board.cells[move.dst]] +
                    CAPTURE_ATTACKER_COST[board.cells[move.src]];
    }
    std::sort(moves, moves + count,
              [&](const q_core::Move lhs, const q_core::Move rhs) { return lhs.info > rhs.info; });
}

inline static void SortByHistoryTable(const HistoryTable& history_table, const q_core::Board& board,
                                      q_core::Move* moves, const size_t count) {
    std::sort(moves, moves + count, [&](const q_core::Move lhs, const q_core::Move rhs) {
        return history_table.GetScore(board.cells[lhs.src], lhs) >
               history_table.GetScore(board.cells[rhs.src], rhs);
    });
}

MovePicker::Stage GetNextStage(MovePicker::Stage stage) {
    return static_cast<MovePicker::Stage>(static_cast<uint8_t>(stage) + 1);
}

MovePicker::MovePicker(const Position& position, const q_core::Move tt_move,
                       const KillerMoves& killer_moves, const HistoryTable& history_table)
    : position_(position),
      tt_move_(tt_move),
      killer_moves_(killer_moves),
      history_table_(history_table),
      movegen_(position.board) {}

q_core::Move MovePicker::GetNextMove() {
    GetNewMoves();
    if (Q_UNLIKELY(stage_ == Stage::End)) {
        return q_core::NULL_MOVE;
    }
    while (stage_ != Stage::TTMove && list_.moves[pos_] == tt_move_) {
        pos_++;
        if (pos_ == list_.size) {
            GetNewMoves();
            if (pos_ == list_.size) {
                return q_core::NULL_MOVE;
            }
        }
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
                if (q_core::IsMovePseudolegal(position_.board, tt_move_)) {
                    list_.moves[list_.size] = tt_move_;
                    list_.size++;
                }
                break;
            }
            case Stage::Capture: {
                const size_t list_old_size = list_.size;
                movegen_.GenerateAllCaptures(position_.board, list_);
                SortByMVVLVA(position_.board, list_.moves + list_old_size,
                             list_.size - list_old_size);
                break;
            }
            case Stage::Promotion: {
                movegen_.GenerateAllPromotions(position_.board, list_);
                break;
            }
            case Stage::KillerMoves: {
                for (size_t i = 0; i < KillerMoves::COUNT; i++) {
                    if (q_core::IsMovePseudolegal(position_.board, killer_moves_.GetMove(i))) {
                        list_.moves[list_.size] = killer_moves_.GetMove(i);
                        list_.size++;
                    }
                }
                break;
            }
            case Stage::History: {
                const size_t list_old_size = list_.size;
                movegen_.GenerateAllSimpleMoves(position_.board, list_);
                for (size_t i = list_old_size; i < list_.size; i++) {
                    bool is_killer_move = false;
                    for (size_t j = 0; j < KillerMoves::COUNT; j++) {
                        if (list_.moves[i] == killer_moves_.GetMove(j)) {
                            is_killer_move = true;
                            break;
                        }
                    }
                    if (is_killer_move) {
                        std::swap(list_.moves[i], list_.moves[list_.size - 1]);
                        list_.size--;
                    }
                }
                SortByHistoryTable(history_table_, position_.board, list_.moves + list_old_size,
                                   list_.size - list_old_size);
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

QuiescenseMovePicker::QuiescenseMovePicker(const Position& position, bool in_check)
    : position_(position), movegen_(position.board), in_check_(in_check) {}

q_core::Move QuiescenseMovePicker::GetNextMove() {
    GetNewMoves();
    if (Q_UNLIKELY(stage_ == Stage::End)) {
        return q_core::NULL_MOVE;
    }
    if (stage_ == Stage::Capture) {
        for (size_t i = pos_ + 1; i < list_.size; i++) {
            if (list_.moves[i].info > list_.moves[pos_].info) {
                std::swap(list_.moves[i], list_.moves[pos_]);
            }
        }
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
                movegen_.GenerateAllCaptures(position_.board, list_);
                AnnotateByMVVLVA(position_.board, list_.moves + list_old_size,
                                 list_.size - list_old_size);
                break;
            }
            case Stage::Promotion: {
                movegen_.GenerateAllPromotions(position_.board, list_);
                break;
            }
            case Stage::Evasions: {
                if (in_check_) {
                    movegen_.GenerateAllSimpleMoves(position_.board, list_);
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

QuiescenseMovePicker::Stage QuiescenseMovePicker::GetStage() const { return stage_; }

}  // namespace q_search
