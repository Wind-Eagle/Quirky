#ifndef QUIRKY_SRC_EVAL_EVAL_H
#define QUIRKY_SRC_EVAL_EVAL_H

#include "../core/board/board.h"
#include "../core/moves/move.h"
#include "../core/util.h"
#include "model.h"
#include "psq.h"

namespace q_eval {

enum class EvaluationType : uint8_t { Value = 0, Vector = 1 };

template <EvaluationType t>
struct EvaluationResultType {
    typedef ScorePair type;
};

struct FeatureStorage {
  public:
    FeatureStorage& operator+=(const FeatureStorage& rhs) {
        for (size_t i = 0; i < FEATURE_COUNT; i++) {
            features_[i] += rhs.features_[i];
        }
        return (*this);
    }
    FeatureStorage& operator-=(const FeatureStorage& rhs) {
        for (size_t i = 0; i < FEATURE_COUNT; i++) {
            features_[i] -= rhs.features_[i];
        }
        return (*this);
    }
    FeatureStorage operator+(const FeatureStorage& rhs) {
        FeatureStorage ans;
        for (size_t i = 0; i < FEATURE_COUNT; i++) {
            ans.features_[i] = features_[i] + rhs.features_[i];
        }
        return ans;
    }
    FeatureStorage operator-(const FeatureStorage& rhs) {
        FeatureStorage ans;
        for (size_t i = 0; i < FEATURE_COUNT; i++) {
            ans.features_[i] = features_[i] - rhs.features_[i];
        }
        return ans;
    }
    int16_t& operator[](const int16_t index) {
        Q_ASSERT(index >= 0 && index < FEATURE_COUNT + PSQ_SIZE);
        return features_[index];
    }
    bool operator == (const FeatureStorage& rhs) const {
        return features_ == rhs.features_;
    }
    const std::array<score_t, FEATURE_COUNT + PSQ_SIZE>& GetFeatures() const {
        return features_;
    }

  private:
    std::array<score_t, FEATURE_COUNT + PSQ_SIZE> features_;
};

template <>
struct EvaluationResultType<EvaluationType::Vector> {
    typedef FeatureStorage type;
};

template <EvaluationType type>
struct Evaluator {
  public:
    struct Tag {
      public:
        void BuildTag(const q_core::Board& board);
        Tag UpdateTag(const q_core::Board& board, const q_core::Move move);

        typename EvaluationResultType<type>::type GetScore() const {
            return score_;
        }

        stage_t GetStage() const {
            return stage_;
        }

        bool operator == (const Tag& rhs) const {
            return score_ == rhs.score_ && stage_ == rhs.stage_;
        }

        static constexpr stage_t CELL_STAGE_EVAL[q_core::NUMBER_OF_CELLS] = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};
        Q_STATIC_ASSERT(CELL_STAGE_EVAL[q_core::EMPTY_CELL] == 0);
        Q_STATIC_ASSERT(CELL_STAGE_EVAL[q_core::MakeCell(q_core::Color::White, q_core::Piece::Pawn)] == 0);
        Q_STATIC_ASSERT(CELL_STAGE_EVAL[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Pawn)] == 0);
        static constexpr uint8_t STAGE_MAX = 24;

      private:
        typename EvaluationResultType<type>::type score_{};
        stage_t stage_{};
    };

    void StartTrackingBoard(const q_core::Board& board) {
        tag_.BuildTag(board);
    }

    void UpdateOnMove(const q_core::Board& board, const q_core::Move move) {
        tag_.UpdateTag(board, move);
    }

    void ReplaceTag(const Tag new_tag) { tag_ = new_tag; }

    typename EvaluationResultType<type>::type Evaluate(const q_core::Board& board) const;

    score_t GetEvaluationScore(typename EvaluationResultType<type>::type score) const;

  private:
    Tag tag_{};
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_EVAL_H
