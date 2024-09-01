#include "calcer.h"

#include <cmath>

#include "../../../src/search/position/move_picker.h"
#include "../../../src/search/position/position.h"
#include "dataset.h"
#include "reader.h"

double GetProbabilityFromScore(const q_eval::score_t score) {
    constexpr double PROB_SIGMOID_SCALE = 1.0475;
    return 1.0 / (1 + std::pow(10, PROB_SIGMOID_SCALE * -score / 400.0));
}

#define MAKE_MOVE(position, move)                                           \
    q_core::MakeMoveInfo _make_move_info;                                   \
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag _evaluator_tag;   \
    bool _legal = position.MakeMove(move, _make_move_info, _evaluator_tag); \
    if (!_legal) {                                                          \
        continue;                                                           \
    }                                                                       \
    Q_DEFER { position.UnmakeMove(move, _make_move_info, _evaluator_tag); }

q_eval::score_t FakeQuiescenseSearch(q_search::Position& position, q_eval::score_t alpha,
                                     q_eval::score_t beta) {
    const q_eval::score_t score = position.GetEvaluatorScore();
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
        return beta;
    }
    q_search::QuiescenseMovePicker move_picker(position);
    for (q_core::Move move = move_picker.GetNextMove();
         move_picker.GetStage() != q_search::QuiescenseMovePicker::Stage::End;
         move = move_picker.GetNextMove()) {
        MAKE_MOVE(position, move);
        q_eval::score_t new_score = -FakeQuiescenseSearch(position, -beta, -alpha);
        alpha = std::max(alpha, new_score);
        if (alpha >= beta) {
            return beta;
        }
    }
    return alpha;
}

bool WillScoreBeUpdated(q_search::Position& position, CalcerParams params,
                        std::shared_ptr<Element> element) {
    if (element->score_type != PositionScoreType::Ready) {
        return true;
    }
    switch (params.change_type) {
        case CalcerFeatureChangeType::PieceWeight: {
            return params.changed_piece != q_core::Piece::King;
        }
        case CalcerFeatureChangeType::PSQ: {
            if (position.board.en_passant_coord != q_core::NO_ENPASSANT_COORD &&
                (params.changed_coord == position.board.en_passant_coord ||
                 q_core::FlipCoord(params.changed_coord) == position.board.en_passant_coord)) {
                return true;
            }
            if (params.changed_piece != q_core::Piece::Pawn &&
                (q_core::GetRank(params.changed_coord) == 0 ||
                 q_core::GetRank(params.changed_coord) == q_core::BOARD_SIDE - 1)) {
                return true;
            }
            return position.board.cells[params.changed_coord] ||
                   position.board.cells[q_core::FlipCoord(params.changed_coord)];
        }
        case CalcerFeatureChangeType::Model: {
            return true;
        }
        default:
            Q_UNREACHABLE();
    }
    return true;
}

double GetCalcerResult(std::shared_ptr<Element> element, CalcerParams params) {
    if (element->score_type == UpdatedJustBefore) {
        if (params.revert_last_change) {
            element->new_score = element->old_score;
            element->old_score = q_eval::SCORE_UNKNOWN;
            if (element->new_score == q_eval::SCORE_UNKNOWN) {
                element->score_type = NotReady;
            } else {
                element->score_type = Ready;
            }
        } else {
            element->score_type = Ready;
        }
    }
    q_search::Position& position = element->position;
    double target = element->result == Result::WhiteWins   ? 1
                    : element->result == Result::BlackWins ? 0
                                                           : 0.5;
    q_eval::score_t score;
    if (WillScoreBeUpdated(position, params, element)) {
        position.evaluator.StartTrackingBoard(position.board);
        score = FakeQuiescenseSearch(position, q_eval::SCORE_MIN, q_eval::SCORE_MAX);
        element->score_type = UpdatedJustBefore;
        element->old_score = element->new_score;
        element->new_score = score;
    } else {
        score = element->new_score;
    }
    if (position.board.move_side == q_core::Color::Black) {
        score *= -1;
    }
    const double prob = GetProbabilityFromScore(score);
    double loss = (prob - target) * (prob - target);
    return loss;
}
