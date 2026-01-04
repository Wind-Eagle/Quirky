#include "move_picker.h"

#include <algorithm>

#include "core/board/board.h"
#include "core/board/types.h"
#include "core/moves/attack.h"
#include "core/moves/move.h"
#include "core/moves/movegen.h"
#include "core/util.h"
#include "position.h"
#include "util/io.h"
#include "util/macro.h"

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

void KillerMoves::Update(const q_core::Move best_move) {
    if (!q_core::IsMoveCapture(best_move) && !q_core::IsMovePromotion(best_move)) {
        Add(best_move);
    }
}

HistoryTable::HistoryTable() {
    for (q_core::cell_t i = 0; i < q_core::BOARD_SIZE; i++) {
        for (q_core::coord_t j = 0; j < q_core::BOARD_SIZE; j++) {
            table_[0][i][j] = 0;
            table_[1][i][j] = 0;
        }
    }

    for (q_core::cell_t i = 0; i < q_core::NUMBER_OF_CELLS; i++) {
        for (q_core::coord_t j = 0; j < q_core::BOARD_SIZE; j++) {
            for (q_core::cell_t k = 0; k < q_core::NUMBER_OF_PIECES; k++) {
                capture_table_[i][j][k] = 0;
            }
        }
    }
}

// History bonuses are a mixture of ideas used in
// avalanche and berserk chess engines
// https://github.com/SnowballSH/Avalanche/blob/master/src/engine/search.zig
// https://github.com/jhonnold/berserk/blob/main/src/history.h

int16_t& HistoryTable::GetQuietEntry(const q_core::Board& board, const q_core::Move move) {
    return table_[static_cast<size_t>(board.move_side)][move.src][move.dst];
}

int16_t& HistoryTable::GetCaptureEntry(const q_core::Board& board, const q_core::Move move) {
    const q_core::cell_t src = board.cells[move.src];
    const q_core::Piece dst = board.cells[move.dst] == q_core::EMPTY_CELL
                                  ? q_core::Piece::Pawn
                                  : q_core::GetCellPiece(board.cells[move.dst]);
    return capture_table_[src][move.dst][static_cast<size_t>(dst) - 1];
}

const int16_t& HistoryTable::GetQuietEntry(const q_core::Board& board,
                                           const q_core::Move move) const {
    return table_[static_cast<size_t>(board.move_side)][move.src][move.dst];
}

const int16_t& HistoryTable::GetCaptureEntry(const q_core::Board& board,
                                             const q_core::Move move) const {
    const q_core::cell_t src = board.cells[move.src];
    const q_core::Piece dst = board.cells[move.dst] == q_core::EMPTY_CELL
                                  ? q_core::Piece::Pawn
                                  : q_core::GetCellPiece(board.cells[move.dst]);
    return capture_table_[src][move.dst][static_cast<size_t>(dst) - 1];
}

void HistoryTable::UpdateQuiet(const q_core::Board& board, const q_core::Move move, const int adj) {
    int16_t& entry = GetQuietEntry(board, move);
    entry += adj - entry * std::abs(adj) / 16384;
}

void HistoryTable::UpdateCapture(const q_core::Board& board, q_core::Move move, const int adj) {
    int16_t& entry = GetCaptureEntry(board, move);
    entry += adj - entry * std::abs(adj) / 16384;
}

int16_t HistoryTable::GetQuietScore(const q_core::Board& board, const q_core::Move move) const {
    return GetQuietEntry(board, move);
}

int16_t HistoryTable::GetCaptureScore(const q_core::Board& board, const q_core::Move move) const {
    return GetCaptureEntry(board, move);
}

void HistoryTable::Update(const Position& position, depth_t depth, q_core::Move best_move,
                          const q_core::MoveList& captures, const q_core::MoveList& quiet_moves) {
    const int adj = std::min(1708, 4 * depth * depth + 191 * depth - 118);

    if (q_core::IsMoveCapture(best_move)) {
        UpdateCapture(position.board, best_move, adj);
    } else if (!q_core::IsMoveCapture(best_move) && !q_core::IsMovePromotion(best_move)) {
        UpdateQuiet(position.board, best_move, adj);
    }

    for (size_t i = 0; i + 1 < captures.size; i++) {
        UpdateCapture(position.board, captures.moves[i], -adj);
    }
    if (!q_core::IsMoveCapture(best_move) && !q_core::IsMovePromotion(best_move)) {
        for (size_t i = 0; i + 1 < quiet_moves.size; i++) {
            UpdateQuiet(position.board, quiet_moves.moves[i], -adj);
        }
    }
}

inline static void SortMoves(q_core::Move* moves, const std::array<int, 256>& scores,
                             const size_t count) {
    Q_ASSERT(count < 256);
    for (size_t i = 0; i < count; ++i) {
        moves[i].info = i;
    }
    std::sort(moves, moves + count, [&](const q_core::Move lhs, const q_core::Move rhs) {
        return scores[lhs.info] > scores[rhs.info];
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

#define SKIP_MOVE                     \
    pos_++;                           \
    if (pos_ == list_.size) {         \
        GetNewMoves();                \
        if (pos_ == list_.size) {     \
            return q_core::NULL_MOVE; \
        }                             \
    }

bool IsCaptureGood(const q_core::Board& board, const q_core::Move move,
                   const std::array<int, 256>& scores, size_t pos) {
    return q_core::IsSEENotNegative(board, move, -scores[pos] / 2, SEE_CELLS_VALUE) &&
           !(q_core::IsMovePromotion(move) &&
             q_core::GetPromotionPiece(move) != q_core::Piece::Queen);
}

bool IsQuietGood(const q_core::Move move) {
    return !(q_core::IsMovePromotion(move) && q_core::GetPromotionPiece(move) != q_core::Piece::Queen);
}

q_core::Move MovePicker::GetNextMove() {
    GetNewMoves();
    if (Q_UNLIKELY(stage_ == Stage::End)) {
        return q_core::NULL_MOVE;
    }
    while (stage_ != Stage::TTMove && list_.moves[pos_] == tt_move_) {
        SKIP_MOVE;
    }
    while (stage_ == Stage::Capture) {
        if (IsCaptureGood(position_.board, list_.moves[pos_], scores_, pos_ - stage_start_pos_)) {
            break;
        }
        bad_list_.moves[bad_list_.size++] = list_.moves[pos_];
        SKIP_MOVE;
    }
    return list_.moves[pos_++];
}

bool IsValidKiller(const q_core::Board& board, const q_core::Move move) {
    return q_core::IsMovePseudolegal(board, move) && board.cells[move.dst] == q_core::EMPTY_CELL;
}

void ScoreCaptures(const q_core::Board& board, const HistoryTable& history_table,
                   q_core::Move* moves, std::array<int, 256>& scores, const size_t count) {
    for (size_t i = 0; i < count; i++) {
        scores[i] = history_table.GetCaptureScore(board, moves[i]) / 16 +
                    SEE_CELLS_VALUE[board.cells[moves[i].dst]];
    }
}

void ScoreQuiets(const q_core::Board& board, const HistoryTable& history_table, q_core::Move* moves,
                 std::array<int, 256>& scores, const size_t count) {
    for (size_t i = 0; i < count; i++) {
        scores[i] = history_table.GetQuietScore(board, moves[i]);
    }
}

void MovePicker::GetNewMoves() {
    while (pos_ == list_.size) {
        if (stage_ != Stage::End) {
            stage_ = GetNextStage(stage_);
            stage_start_pos_ = pos_;
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
                ScoreCaptures(position_.board, history_table_, list_.moves + list_old_size, scores_,
                              list_.size - list_old_size);
                SortMoves(list_.moves + list_old_size, scores_, list_.size - list_old_size);
                break;
            }
            case Stage::Promotion: {
                movegen_.GenerateAllPromotions(position_.board, list_);
                break;
            }
            case Stage::KillerMoves: {
                for (size_t i = 0; i < KillerMoves::COUNT; i++) {
                    if (IsValidKiller(position_.board, killer_moves_.GetMove(i))) {
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
                ScoreQuiets(position_.board, history_table_, list_.moves + list_old_size, scores_,
                            list_.size - list_old_size);
                SortMoves(list_.moves + list_old_size, scores_, list_.size - list_old_size);
                break;
            }
            case Stage::Bad: {
                const size_t list_old_size = list_.size;
                list_.size += bad_list_.size;
                for (size_t i = 0; i < bad_list_.size; i++) {
                    list_.moves[i + list_old_size] = bad_list_.moves[i];
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

QuiescenseMovePicker::QuiescenseMovePicker(const Position& position, bool in_check,
                                           const HistoryTable& history_table)
    : position_(position),
      movegen_(position.board),
      in_check_(in_check),
      history_table_(history_table) {}

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
                movegen_.GenerateAllCaptures(position_.board, list_);
                ScoreCaptures(position_.board, history_table_, list_.moves + list_old_size, scores_,
                              list_.size - list_old_size);
                SortMoves(list_.moves + list_old_size, scores_, list_.size - list_old_size);
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
