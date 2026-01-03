#ifndef QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H
#define QUIRKY_SRC_SEARCH_POSITION_MOVE_PICKER_H

#include "core/board/board.h"
#include "core/moves/movegen.h"
#include "core/board/types.h"
#include "position.h"

namespace q_search {

class KillerMoves {
  public:
    static constexpr uint8_t COUNT = 2;
    KillerMoves();
    q_core::Move GetMove(uint8_t index) const;
    void Update(q_core::Move best_move);

  private:
    void Add(q_core::Move move);
    std::array<q_core::Move, COUNT> moves_;
};

class HistoryTable {
  public:
    HistoryTable();
    void Update(const Position& position, depth_t depth, q_core::Move best_move, const q_core::MoveList& captures, const q_core::MoveList& quiet_moves);
    int16_t GetQuietScore(const q_core::Board& board, q_core::Move move) const;
    int16_t GetCaptureScore(const q_core::Board& board, q_core::Move move) const;

  private:
    int16_t& GetQuietEntry(const q_core::Board& board, q_core::Move move);
    int16_t& GetCaptureEntry(const q_core::Board& board, q_core::Move move);
    const int16_t& GetQuietEntry(const q_core::Board& board, q_core::Move move) const;
    const int16_t& GetCaptureEntry(const q_core::Board& board, q_core::Move move) const;
    void UpdateQuiet(const q_core::Board& board, q_core::Move move, int adj);
    void UpdateCapture(const q_core::Board& board, q_core::Move move, int adj);
    std::array<std::array<std::array<int16_t, q_core::BOARD_SIZE>, q_core::BOARD_SIZE>, 2> table_;
    std::array<std::array<std::array<int16_t, q_core::NUMBER_OF_PIECES>, q_core::BOARD_SIZE>, q_core::NUMBER_OF_PIECES> capture_table_;
};

inline static constexpr std::array<int16_t, q_core::NUMBER_OF_CELLS> SEE_CELLS_VALUE = {
    0, 100, 300, 300, 500, 900, 3000, 100, 300, 300, 500, 900, 3000};

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
    std::array<int, 256> scores_;
    q_core::Move tt_move_;
    const KillerMoves& killer_moves_;
    const HistoryTable& history_table_;
    q_core::Movegen movegen_;
    size_t pos_ = 0;
    Stage stage_ = Stage::Start;
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
