#ifndef QUIRKY_SRC_SEARCH_POSITION_POSITION_H
#define QUIRKY_SRC_SEARCH_POSITION_POSITION_H

#include <functional>
#include <string_view>

#include "core/board/board.h"
#include "core/moves/board_manipulation.h"
#include "eval/evaluator.h"
#include "eval/score.h"

namespace q_search {

using depth_t = int16_t;
using idepth_t = uint16_t;

struct Position {
    public:
        q_core::Board board;
        q_eval::Evaluator evaluator;

        explicit Position(const q_core::Board& b);
        explicit Position(std::string_view s);

        Position(const Position&) = delete;

        ~Position();

        static constexpr size_t MAX_BUFFER_SIZE = 256;

        bool MakeMove(q_core::Move move, q_core::MakeMoveInfo& make_move_info);
        bool MakeMove(q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                    const std::function<void()>& after_board_change);
        void UnmakeMove(q_core::Move move, const q_core::MakeMoveInfo& make_move_info);

        void MakeNullMove(q_core::coord_t& old_en_passant_coord);
        void UnmakeNullMove(const q_core::coord_t& old_en_passant_coord);

        bool HasNonPawns() const;
        bool HasNonPawns(q_core::Color c) const;
        bool IsCheck() const;

        void PrefetchEvaluatorCache();
        q_eval::score_t GetEvaluatorScore();

    private:
      struct EvaluatorCache {
        struct Entry {
            uint32_t hash_first;
            uint16_t hash_second;
            q_eval::score_t score = q_eval::SCORE_UNKNOWN;
        };

        void Store(const q_core::Board& board, q_eval::score_t score);
        q_eval::score_t Load(const q_core::Board& board) const;
        void Prefetch(const q_core::Board& board);

        static constexpr uint8_t EVALUATOR_CACHE_SIZE_LOG = 16;
        std::array<Entry, (1 << EVALUATOR_CACHE_SIZE_LOG)> data{};
      };

      void ConstructPosition();
      EvaluatorCache cache_;
      alignas(64) std::array<q_eval::Evaluator::State*, MAX_BUFFER_SIZE> buffer_;
      [[maybe_unused]]size_t buffer_head_ = 0;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_POSITION_POSITION_H
