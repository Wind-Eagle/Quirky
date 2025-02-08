#ifndef QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
#define QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H

#include "../../core/moves/movegen.h"
#include "position.h"

namespace q_search {

class KillerMoves {
  public:
    static constexpr uint8_t COUNT = 2;
    KillerMoves();
    void Add(q_core::Move move);
    q_core::Move GetMove(uint8_t index) const;

  private:
    std::array<q_core::Move, COUNT> moves_;
};

class HistoryTable {
  public:
    HistoryTable();
    void Update(q_core::cell_t cell, q_core::Move move, depth_t depth);
    uint64_t GetScore(q_core::cell_t cell, q_core::Move move) const;

  private:
    std::array<std::array<uint64_t, q_core::BOARD_SIZE>, q_core::NUMBER_OF_CELLS> table_;
};

class MovePicker {
  public:
    MovePicker(const Position& position, q_core::Move tt_move, const KillerMoves& killer_moves,
               const HistoryTable& history_table);
    enum class Stage : uint8_t {
        Start = 0,
        TTMove = 1,
        Capture = 2,
        Promotion = 3,
        KillerMoves = 4,
        History = 5,
        End = 6
    };
    q_core::Move GetNextMove();
    Stage GetStage() const;

  private:
    void GetNewMoves();
    const Position& position_;
    q_core::MoveList list_;
    q_core::Move tt_move_;
    const KillerMoves& killer_moves_;
    const HistoryTable& history_table_;
    q_core::Movegen movegen_;
    size_t pos_ = 0;
    Stage stage_ = Stage::Start;
};

class QuiescenseMovePicker {
  public:
    explicit QuiescenseMovePicker(const Position& position);
    enum class Stage : uint8_t { Start = 0, Capture = 1, Promotion = 2, End = 3 };
    q_core::Move GetNextMove();
    Stage GetStage() const;

  private:
    void GetNewMoves();
    const Position& position_;
    q_core::MoveList list_;
    q_core::Movegen movegen_;
    size_t pos_ = 0;
    Stage stage_ = Stage::Start;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
