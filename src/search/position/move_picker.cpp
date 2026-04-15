#include "move_picker.h"

#include <algorithm>

#include "core/board/board.h"
#include "core/board/types.h"
#include "core/moves/attack.h"
#include "core/moves/move.h"
#include "core/moves/movegen.h"
#include "core/util.h"
#include "position.h"
#include "util/macro.h"

namespace q_search {

HistoryTable::KillerMoves::KillerMoves() {
    for (size_t i = 0; i < COUNT; i++) {
        moves_[i] = q_core::NULL_MOVE;
    }
}

void HistoryTable::KillerMoves::Add(const q_core::Move move) {
    for (size_t j = COUNT - 1; j >= 1; j--) {
        moves_[j] = moves_[j - 1];
    }
    moves_[0] = move;
}

q_core::Move HistoryTable::KillerMoves::GetMove(const uint8_t index) const { return moves_[index]; }

HistoryTable::HistoryTable() {
    for (size_t i = 0; i < 256; i++) {
        killer_moves_[i] = KillerMoves();
    }

    for (q_core::cell_t i = 0; i < q_core::BOARD_SIZE; i++) {
        for (q_core::coord_t j = 0; j < q_core::NUMBER_OF_CELLS; j++) {
            counter_moves_[i][j] = q_core::NULL_MOVE;
        }
    }

    for (q_core::cell_t i = 0; i < q_core::BOARD_SIZE; i++) {
        for (q_core::coord_t j = 0; j < q_core::BOARD_SIZE; j++) {
            simple_table_[0][i][j] = 0;
            simple_table_[1][i][j] = 0;
        }
    }

    for (q_core::cell_t i = 0; i < q_core::NUMBER_OF_CELLS; i++) {
        for (q_core::coord_t j = 0; j < q_core::BOARD_SIZE; j++) {
            for (q_core::cell_t k = 0; k < q_core::NUMBER_OF_PIECES; k++) {
                capture_table_[i][j][k] = 0;
            }
        }
    }

    for (q_core::cell_t i = 0; i < q_core::BOARD_SIZE; i++) {
        for (q_core::coord_t j = 0; j < q_core::NUMBER_OF_CELLS; j++) {
            for (q_core::cell_t k = 0; k < q_core::BOARD_SIZE; k++) {
                for (q_core::coord_t l = 0; l < q_core::NUMBER_OF_CELLS; l++) {
                    continuation_table_[0][i][j][k][l] = 0;
                    continuation_table_[1][i][j][k][l] = 0;
                }
            }
        }
    }
}

// History bonuses are a mixture of ideas used in
// avalanche and berserk chess engines
// https://github.com/SnowballSH/Avalanche/blob/master/src/engine/search.zig
// https://github.com/jhonnold/berserk/blob/main/src/history.h

int16_t& HistoryTable::GetSimpleEntry(const q_core::Board& board, const q_core::Move move) {
    return simple_table_[static_cast<size_t>(board.move_side)][move.src][move.dst];
}

int16_t& HistoryTable::GetCaptureEntry(const q_core::Board& board, const q_core::Move move) {
    const q_core::cell_t src = board.cells[move.src];
    const q_core::Piece dst = board.cells[move.dst] == q_core::EMPTY_CELL
                                  ? q_core::Piece::Pawn
                                  : q_core::GetCellPiece(board.cells[move.dst]);
    return capture_table_[src][move.dst][static_cast<size_t>(dst) - 1];
}

int16_t& HistoryTable::GetContinuationEntry(const q_core::Board& board,
                                           const q_core::Move move, const StatefulMove prev_move) {
    return continuation_table_[q_core::IsMoveCapture(prev_move.move)][prev_move.move.dst][prev_move.cell][move.dst][board.cells[move.src]];
}

const int16_t& HistoryTable::GetSimpleEntry(const q_core::Board& board,
                                           const q_core::Move move) const {
    return simple_table_[static_cast<size_t>(board.move_side)][move.src][move.dst];
}

const int16_t& HistoryTable::GetCaptureEntry(const q_core::Board& board,
                                             const q_core::Move move) const {
    const q_core::cell_t src = board.cells[move.src];
    const q_core::Piece dst = board.cells[move.dst] == q_core::EMPTY_CELL
                                  ? q_core::Piece::Pawn
                                  : q_core::GetCellPiece(board.cells[move.dst]);
    return capture_table_[src][move.dst][static_cast<size_t>(dst) - 1];
}

const int16_t& HistoryTable::GetContinuationEntry(const q_core::Board& board,
                                           const q_core::Move move, const StatefulMove prev_move) const {
    return continuation_table_[q_core::IsMoveCapture(prev_move.move)][prev_move.move.dst][prev_move.cell][move.dst][board.cells[move.src]];
}

inline void AddToEntry(int16_t& entry, int adj) {
    entry += adj - entry * std::abs(adj) / 16384;
}

HistoryTable::KillerMoves HistoryTable::GetAllKillerMoves(const AdditionalKeyInfo& info) const {
    return killer_moves_[info.idepth];
}

q_core::Move HistoryTable::GetKillerMove(size_t index, const AdditionalKeyInfo& info) const {
    return killer_moves_[info.idepth].GetMove(index);
}

q_core::Move HistoryTable::GetCounterMove(const AdditionalKeyInfo& info) const {
    return counter_moves_[info.prev_moves[0].move.dst][info.prev_moves[0].cell];
}

void HistoryTable::UpdateKillerMove(q_core::Move move, const AdditionalKeyInfo& info) {
    killer_moves_[info.idepth].Add(move);
}

void HistoryTable::UpdateCounterMove(q_core::Move move, const AdditionalKeyInfo& info) {
    counter_moves_[info.prev_moves[0].move.dst][info.prev_moves[0].cell] = move;
}


void HistoryTable::UpdateQuiet(const q_core::Board& board, const q_core::Move move, const AdditionalKeyInfo& info, const int adj) {
    AddToEntry(GetSimpleEntry(board, move), adj);

    const auto add_to_ch = [&](size_t index) {
        if (!q_core::IsMoveNull(info.prev_moves[index].move)) {
            AddToEntry(GetContinuationEntry(board, move, info.prev_moves[index]), adj);
        }
    };

    add_to_ch(0);
    add_to_ch(1);
    add_to_ch(3);
}

void HistoryTable::UpdateCapture(const q_core::Board& board, q_core::Move move, const int adj) {
    AddToEntry(GetCaptureEntry(board, move), adj);
}

int HistoryTable::GetQuietScore(const q_core::Board& board, const q_core::Move move, const AdditionalKeyInfo& info) const {
    const int simple_score = GetSimpleEntry(board, move);
    int res = simple_score;

    const auto add_ch = [&](size_t index) {
        res += !q_core::IsMoveNull(info.prev_moves[index].move) ? GetContinuationEntry(board, move, info.prev_moves[index]) : simple_score;
    };

    add_ch(0);
    add_ch(1);
    add_ch(3);

    return res;
}

int HistoryTable::GetCaptureScore(const q_core::Board& board, const q_core::Move move) const {
    return GetCaptureEntry(board, move);
}

void HistoryTable::Update(const q_core::Board& board, q_core::Move best_move, const AdditionalKeyInfo& info) {
    if (IsMoveQuiet(best_move)) {
        UpdateKillerMove(best_move, info);
        if (!IsMoveNull(info.prev_moves[0].move)) {
            UpdateCounterMove(best_move, info);
        }
    }

    const int adj = std::min(1708, 4 * info.depth * info.depth + 191 * info.depth - 118);

    if (q_core::IsMoveCapture(best_move)) {
        UpdateCapture(board, best_move, adj);
    } else if (IsMoveQuiet(best_move)) {
        UpdateQuiet(board, best_move, info, adj);
    }

    for (size_t i = 0; i + 1 < info.captures.size; i++) {
        UpdateCapture(board, info.captures.moves[i], -adj);
    }
    if (IsMoveQuiet(best_move)) {
        for (size_t i = 0; i + 1 < info.quiets.size; i++) {
            UpdateQuiet(board, info.quiets.moves[i], info, -adj);
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
                       const HistoryTable& history_table, const HistoryTable::AdditionalKeyInfo& history_info)
    : position_(position),
      tt_move_(tt_move),
      history_table_(history_table),
      history_info_(history_info),
      movegen_(position.board) {
        killer_moves_ = history_table_.GetAllKillerMoves(history_info_);
        counter_move_ = q_core::NULL_MOVE;
        if (!IsMoveNull(history_info_.prev_moves[0].move)) {
            counter_move_ = history_table_.GetCounterMove(history_info_);
        }
      }

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

void MovePicker::SkipQuiets() {
    skip_quiets_ = true;
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
    while (skip_quiets_ && IsMoveQuiet(list_.moves[pos_])) {
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
                    SEE_CELLS_VALUE[board.cells[moves[i].dst]] + (q_core::IsMovePromotion(moves[i]) ? 1024 : 0);
    }
}

void ScoreQuiets(const q_core::Board& board, const HistoryTable& history_table, const HistoryTable::AdditionalKeyInfo& history_info, q_core::Move* moves,
                 std::array<int, 256>& scores, const size_t count) {
    for (size_t i = 0; i < count; i++) {
        scores[i] = history_table.GetQuietScore(board, moves[i], history_info);
    }
}

bool MovePicker::IsKillerMove(const q_core::Move move) const {
    for (size_t i = 0; i < HistoryTable::KillerMoves::COUNT; i++) {
        if (move == killer_moves_.GetMove(i)) {
            return true;
        }
    }
    return false;
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
                const size_t list_old_size = list_.size;
                movegen_.GenerateAllPromotions(position_.board, list_);
                if (list_.size != list_old_size) {
                    auto* ptr = std::partition(list_.moves + list_old_size, list_.moves + list_.size, [&](const q_core::Move move){
                        return q_core::GetPromotionPiece(move) == q_core::Piece::Queen;
                    });
                    std::copy(ptr, list_.moves + list_.size, bad_list_.moves + bad_list_.size);
                    const size_t underpromotions_cnt = list_.moves + list_.size - ptr;
                    bad_list_.size += underpromotions_cnt;
                    list_.size -= underpromotions_cnt;
                };
                break;
            }
            case Stage::KillerMoves: {
                if (skip_quiets_) {
                    break;
                }
                for (size_t i = 0; i < HistoryTable::KillerMoves::COUNT; i++) {
                    if (IsValidKiller(position_.board, killer_moves_.GetMove(i))) {
                        list_.moves[list_.size] = killer_moves_.GetMove(i);
                        list_.size++;
                    }
                }
                break;
            }
            case Stage::CounterMove: {
                if (skip_quiets_) {
                    break;
                }
                if (!IsKillerMove(counter_move_) && IsValidKiller(position_.board, counter_move_)) {
                    list_.moves[list_.size] = counter_move_;
                    list_.size++;
                }
                break;
            }
            case Stage::History: {
                if (skip_quiets_) {
                    break;
                }
                const size_t list_old_size = list_.size;
                movegen_.GenerateAllSimpleMoves(position_.board, list_);
                for (size_t i = list_old_size; i < list_.size; i++) {
                    if (IsKillerMove(list_.moves[i]) || list_.moves[i] == counter_move_) {
                        std::swap(list_.moves[i], list_.moves[list_.size - 1]);
                        list_.size--;
                    }
                }
                ScoreQuiets(position_.board, history_table_, history_info_, list_.moves + list_old_size, scores_,
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
