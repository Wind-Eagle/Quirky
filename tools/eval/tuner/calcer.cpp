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

bool WillScoreBeUpdated(q_search::Position& position, CalcerParams params, PositionState state) {
    if (state.score_type != PositionScoreType::Ready) {
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

CalcerResult GetCalcerResult(std::shared_ptr<Element> element, CalcerParams params) {
    const Game& game = element->game;
    q_search::Position position;
    position.MakeFromFEN(game.start_board_fen);
    double target = game.header.result == Result::WhiteWins   ? 1
                    : game.header.result == Result::BlackWins ? 0
                                                              : 0.5;
    double loss = 0;

    q_core::MakeMoveInfo make_move_info;
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag evaluator_tag;
    size_t position_count = 0;
    size_t positions_cache_hits = 0;
    for (size_t i = 0; i < game.moves.size(); i++) {
        if (i > 0) {
            position.MakeMove(q_core::TranslateStringToMove(position.board, game.moves[i]),
                              make_move_info, evaluator_tag);
        }
        if (position.board.move_count <= 12) {
            continue;
        }
        position_count++;

        PositionState& position_state = element->states.at(i);
        if (position_state.score_type == UpdatedJustBefore) {
            if (params.revert_last_change) {
                position_state.new_score = position_state.old_score;
                position_state.old_score = q_eval::SCORE_UNKNOWN;
                if (position_state.new_score == q_eval::SCORE_UNKNOWN) {
                    position_state.score_type = NotReady;
                } else {
                    position_state.score_type = Ready;
                }
            } else {
                position_state.score_type = Ready;
            }
        }
        q_eval::score_t score = 0;
        if (WillScoreBeUpdated(position, params, position_state)) {
            score = FakeQuiescenseSearch(position, q_eval::SCORE_MIN, q_eval::SCORE_MAX);
            position_state.score_type = UpdatedJustBefore;
            position_state.old_score = position_state.new_score;
            position_state.new_score = score;
        } else {
            /*score = FakeQuiescenseSearch(position, q_eval::SCORE_MIN, q_eval::SCORE_MAX);
            if (score != position_state.new_score) {
                std::cerr << position.board.GetFEN() << " " << score << " "
                          << position_state.new_score << " "
                          << static_cast<int>(position_state.score_type) << " "
                          << static_cast<int>(params.changed_piece) << " "
                          << static_cast<int>(params.changed_coord) << " "
                          << static_cast<int>(params.change_type) << " "
                          << params.revert_last_change << std::endl;
                exit(42);
            }*/
            positions_cache_hits++;
            score = position_state.new_score;
        }
        if (position.board.move_side == q_core::Color::Black) {
            score *= -1;
        }
        const double prob = GetProbabilityFromScore(score);
        loss += (prob - target) * (prob - target);
    }
    return {.loss = loss,
            .number_of_positions = position_count,
            .positions_cache_hits = positions_cache_hits};
}
