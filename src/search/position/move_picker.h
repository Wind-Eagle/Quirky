#ifndef QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
#define QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H

#include <cstddef>
#include <cstdint>
#include "core/board/board.h"
#include "core/moves/move.h"
#include "core/moves/movegen.h"
#include "core/board/types.h"
#include "position.h"

namespace q_search {

struct StatefulMove {
  q_core::Move move;
  q_core::cell_t cell;
  int8_t info;
};

inline StatefulMove ConstructStatefulMove(q_core::Move move, q_core::cell_t cell, int8_t info = 0) {
  return {.move = move, .cell = cell, .info = info};
}

inline constexpr bool IsMoveQuiet(const q_core::Move move) {
    return !q_core::IsMoveCapture(move) && !q_core::IsMovePromotion(move);
}

class HistoryTable {
  public:
    struct AdditionalKeyInfo {
      static constexpr uint8_t CH_SIZE = 6;

      std::array<StatefulMove, CH_SIZE> prev_moves;
      q_core::MoveList captures;
      q_core::MoveList quiets;
      depth_t depth;
      idepth_t idepth;
    };
    class KillerMoves {
    public:
      static constexpr uint8_t COUNT = 2;
      KillerMoves();
      q_core::Move GetMove(uint8_t index) const;
      void Add(q_core::Move move);

    private:
      std::array<q_core::Move, COUNT> moves_;
  };

    HistoryTable();
    void Update(const q_core::Board& board, q_core::Move best_move, const AdditionalKeyInfo& info);

    KillerMoves GetAllKillerMoves(const AdditionalKeyInfo& info) const;
    q_core::Move GetKillerMove(size_t index, const AdditionalKeyInfo& info) const;
    q_core::Move GetCounterMove(const AdditionalKeyInfo& info) const;
    int GetQuietScore(const q_core::Board& board, q_core::Move move, const AdditionalKeyInfo& info) const;
    int GetCaptureScore(const q_core::Board& board, q_core::Move move) const;

  private:
    int16_t& GetSimpleEntry(const q_core::Board& board, q_core::Move move);
    int16_t& GetCaptureEntry(const q_core::Board& board, q_core::Move move);
    int16_t& GetContinuationEntry(const q_core::Board& board, q_core::Move move, StatefulMove prev_move);
    const int16_t& GetSimpleEntry(const q_core::Board& board, q_core::Move move) const;
    const int16_t& GetCaptureEntry(const q_core::Board& board, q_core::Move move) const;
    const int16_t& GetContinuationEntry(const q_core::Board& board, q_core::Move move, StatefulMove prev_move) const;
    void UpdateKillerMove(q_core::Move move, const AdditionalKeyInfo& info);
    void UpdateCounterMove(q_core::Move move, const AdditionalKeyInfo& info);
    void UpdateQuiet(const q_core::Board& board, q_core::Move move, const AdditionalKeyInfo& info, int adj);
    void UpdateCapture(const q_core::Board& board, q_core::Move move, int adj);

    std::array<KillerMoves, 256> killer_moves_;
    std::array<std::array<q_core::Move, q_core::NUMBER_OF_CELLS>, q_core::BOARD_SIZE> counter_moves_;
    std::array<std::array<std::array<int16_t, q_core::BOARD_SIZE>, q_core::BOARD_SIZE>, 2> simple_table_;
    std::array<std::array<std::array<int16_t, q_core::NUMBER_OF_PIECES>, q_core::BOARD_SIZE>, q_core::NUMBER_OF_CELLS> capture_table_;
    std::array<std::array<std::array<std::array<std::array<int16_t, q_core::NUMBER_OF_CELLS>, q_core::BOARD_SIZE>, q_core::NUMBER_OF_CELLS>, q_core::BOARD_SIZE>, 2> continuation_table_;
};

// Values are inherited from Obsidian chess engine
// https://github.com/gab8192/Obsidian/blob/main/src/types.h

inline static constexpr std::array<int16_t, q_core::NUMBER_OF_CELLS> SEE_CELLS_VALUE = {
    0, 100, 370, 390, 610, 1210, 3000, 100, 370, 390, 610, 1210, 3000};

class MovePicker {
  public:
    MovePicker(const Position& position, q_core::Move tt_move, const HistoryTable& history_table, const HistoryTable::AdditionalKeyInfo& history_info);
    enum class Stage : uint8_t {
        Start = 0,
        TTMove = 1,
        Capture = 2,
        Promotion = 3,
        KillerMoves = 4,
        CounterMove = 5,
        History = 6,
        Bad = 7,
        End = 8
    };
    void SkipQuiets();
    q_core::Move GetNextMove();
    Stage GetStage() const;

  private:
    bool IsKillerMove(q_core::Move move) const;
    void GetNewMoves();
    const Position& position_;
    q_core::MoveList list_;
    q_core::MoveList bad_list_;
    std::array<int, 256> scores_;
    q_core::Move tt_move_;
    HistoryTable::KillerMoves killer_moves_;
    q_core::Move counter_move_;
    const HistoryTable& history_table_;
    const HistoryTable::AdditionalKeyInfo& history_info_;
    q_core::Movegen movegen_;
    size_t pos_ = 0;
    size_t stage_start_pos_ = 0;
    Stage stage_ = Stage::Start;
    bool skip_quiets_ = false;
};

class QuiescenseMovePicker {
  public:
    QuiescenseMovePicker(const Position& position, bool in_check, const HistoryTable& history_table);
    enum class Stage : uint8_t { Start = 0, Capture = 1, Promotion = 2, Evasions = 3, End = 4 };
    q_core::Move GetNextMove();
    Stage GetStage() const;

  private:
    void GetNewMoves();
    const Position& position_;
    q_core::Movegen movegen_;
    bool in_check_ = false;
    const HistoryTable& history_table_;
    q_core::MoveList list_;
    std::array<int, 256> scores_;
    size_t pos_ = 0;
    Stage stage_ = Stage::Start;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
