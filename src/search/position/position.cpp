#include "position.h"

#include "core/board/types.h"
#include "core/moves/attack.h"
#include "core/util.h"
#include "eval/evaluator.h"
#include "eval/score.h"

namespace q_search {

Position::Position(const q_core::Board& b) {
    board = b;
    ConstructPosition();
}

Position::Position(const std::string_view s) {
    board.MakeFromFEN(s);
    ConstructPosition();
}

Position::~Position() {
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
        delete buffer_[i];
    }
}

void Position::UnmakeMove(const q_core::Move move, const q_core::MakeMoveInfo& make_move_info) {
    evaluator.SetState(buffer_[--buffer_head_]);
    q_core::UnmakeMove(board, move, make_move_info);
}

bool Position::MakeMove(const q_core::Move move, q_core::MakeMoveInfo& make_move_info) {
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        return false;
    }
    PrefetchEvaluatorCache();
    evaluator.UpdateOnMove(board, move, make_move_info, buffer_[++buffer_head_]);
    return true;
}

bool Position::MakeMove(q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        const std::function<void()>& after_board_change) {
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        return false;
    }
    after_board_change();
    PrefetchEvaluatorCache();
    evaluator.UpdateOnMove(board, move, make_move_info, buffer_[++buffer_head_]);
    return true;
}

void Position::MakeNullMove(q_core::coord_t& old_en_passant_coord) {
    q_core::MakeNullMove(board, old_en_passant_coord);
}

void Position::UnmakeNullMove(const q_core::coord_t& old_en_passant_coord) {
    q_core::UnmakeNullMove(board, old_en_passant_coord);
}

void Position::ConstructPosition() {
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
        buffer_[i] = new q_eval::Evaluator::State();
    }
    evaluator.StartTrackingBoard(board, buffer_[0]);
}

bool Position::IsCheck() const { return q_core::IsKingInCheck(board); }

void Position::PrefetchEvaluatorCache() { cache_.Prefetch(board); }

q_eval::score_t Position::GetEvaluatorScore() {
    const q_eval::score_t cache_score = cache_.Load(board);
    if (cache_score != q_eval::SCORE_UNKNOWN) {
        return cache_score;
    }
    const q_eval::score_t score = evaluator.Evaluate(board);
    cache_.Store(board, score);
    return score;
}

bool Position::HasNonPawns() const {
    return board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Knight)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Bishop)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Queen)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Knight)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Bishop)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Rook)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Queen)];
}

bool Position::HasNonPawns(q_core::Color c) const {
    return board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Knight)] |
           board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Bishop)] |
           board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Rook)] |
           board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Queen)];
}

uint32_t GetHashFirstPart(q_core::hash_t hash) { return hash >> 32; }

uint16_t GetHashSecondPart(q_core::hash_t hash) { return (hash >> 16) & ((1 << 16) - 1); }

void Position::EvaluatorCache::Prefetch(const q_core::Board& board) {
    Q_PREFETCH(&data[board.hash & ((1 << EVALUATOR_CACHE_SIZE_LOG) - 1)]);
}

void Position::EvaluatorCache::Store(const q_core::Board& board, q_eval::score_t score) {
    uint32_t first_part = GetHashFirstPart(board.hash);
    uint16_t second_part = GetHashSecondPart(board.hash);
    data[board.hash & ((1 << EVALUATOR_CACHE_SIZE_LOG) - 1)] = {
        .hash_first = first_part, .hash_second = second_part, .score = score};
}

q_eval::score_t Position::EvaluatorCache::Load(const q_core::Board& board) const {
    uint32_t first_part = GetHashFirstPart(board.hash);
    uint16_t second_part = GetHashSecondPart(board.hash);
    const Position::EvaluatorCache::Entry entry =
        data[board.hash & ((1 << EVALUATOR_CACHE_SIZE_LOG) - 1)];
    if (entry.hash_first != first_part || entry.hash_second != second_part) {
        return q_eval::SCORE_UNKNOWN;
    }
    return entry.score;
}

}  // namespace q_search
